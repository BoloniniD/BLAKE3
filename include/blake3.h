#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>


/// The number of bytes in a key, 32.
static const size_t KEY_LEN = 32;

/// The number of bytes in a [`Hash`](struct.Hash.html), 32.
static const size_t OUT_LEN = 32;

enum class Platform {
  Portable,
  SSE2,
  SSE41,
  AVX2,
  AVX512,
  NEON,
};

/// An incremental hash state that can accept any number of writes.
///
/// When the `traits-preview` Cargo feature is enabled, this type implements
/// several commonly used traits from the
/// [`digest`](https://crates.io/crates/digest) and
/// [`crypto_mac`](https://crates.io/crates/crypto-mac) crates. However, those
/// traits aren't stable, and they're expected to change in incompatible ways
/// before those crates reach 1.0. For that reason, this crate makes no SemVer
/// guarantees for this feature, and callers who use it should expect breaking
/// changes between patch versions.
///
/// When the `rayon` Cargo feature is enabled, the
/// [`update_rayon`](#method.update_rayon) method is available for multithreaded
/// hashing.
///
/// **Performance note:** The [`update`](#method.update) method can't take full
/// advantage of SIMD optimizations if its input buffer is too small or oddly
/// sized. Using a 16 KiB buffer, or any multiple of that, enables all currently
/// supported SIMD instruction sets.
///
/// # Examples
///
/// ```
/// # fn main() -> Result<(), Box<dyn std::error::Error>> {
/// // Hash an input incrementally.
/// let mut hasher = blake3::Hasher::new();
/// hasher.update(b"foo");
/// hasher.update(b"bar");
/// hasher.update(b"baz");
/// assert_eq!(hasher.finalize(), blake3::hash(b"foobarbaz"));
///
/// // Extended output. OutputReader also implements Read and Seek.
/// # #[cfg(feature = "std")] {
/// let mut output = [0; 1000];
/// let mut output_reader = hasher.finalize_xof();
/// output_reader.fill(&mut output);
/// assert_eq!(&output[..32], blake3::hash(b"foobarbaz").as_bytes());
/// # }
/// # Ok(())
/// # }
/// ```
struct Hasher;

/// An output of the default size, 32 bytes, which provides constant-time
/// equality checking.
///
/// `Hash` implements [`From`] and [`Into`] for `[u8; 32]`, and it provides an
/// explicit [`as_bytes`] method returning `&[u8; 32]`. However, byte arrays
/// and slices don't provide constant-time equality checking, which is often a
/// security requirement in software that handles private data. `Hash` doesn't
/// implement [`Deref`] or [`AsRef`], to avoid situations where a type
/// conversion happens implicitly and the constant-time property is
/// accidentally lost.
///
/// `Hash` provides the [`to_hex`] and [`from_hex`] methods for converting to
/// and from hexadecimal. It also implements [`Display`] and [`FromStr`].
///
/// [`From`]: https://doc.rust-lang.org/std/convert/trait.From.html
/// [`Into`]: https://doc.rust-lang.org/std/convert/trait.Into.html
/// [`as_bytes`]: #method.as_bytes
/// [`Deref`]: https://doc.rust-lang.org/stable/std/ops/trait.Deref.html
/// [`AsRef`]: https://doc.rust-lang.org/std/convert/trait.AsRef.html
/// [`to_hex`]: #method.to_hex
/// [`from_hex`]: #method.from_hex
/// [`Display`]: https://doc.rust-lang.org/std/fmt/trait.Display.html
/// [`FromStr`]: https://doc.rust-lang.org/std/str/trait.FromStr.html
struct Hash {
  uint8_t _0[OUT_LEN];
};

/// A shim struct containing a pointer to actual Hasher
struct Hasher_shim {
  Hasher *hasher;
};

using CVWords = uint32_t[8];

struct Output {
  CVWords input_chaining_value;
  uint8_t block[64];
  uint8_t block_len;
  uint64_t counter;
  uint8_t flags;
  Platform platform;
};

/// An incremental reader for extended output, returned by
/// [`Hasher::finalize_xof`](struct.Hasher.html#method.finalize_xof).
///
/// Outputs shorter than the default length of 32 bytes (256 bits)
/// provide less security. An N-bit BLAKE3 output is intended to provide
/// N bits of first and second preimage resistance and N/2 bits of
/// collision resistance, for any N up to 256. Longer outputs don't
/// provide any additional security.
///
/// Shorter BLAKE3 outputs are prefixes of longer ones. Explicitly
/// requesting a short output is equivalent to truncating the
/// default-length output. (Note that this is different between BLAKE2
/// and BLAKE3.)
struct OutputReader {
  Output inner;
  uint8_t position_within_block;
};

struct DerivedOut {
  Hasher_shim hasher;
  char *err;
};


extern "C" {

/// Returns uint8_t* with hash bytes.
const uint8_t *as_bytes_shim(const Hash *obj);

/// Returns u64 number of hashed bytes
uint64_t count_shim(Hasher_shim *hasher);

void fill_shim(OutputReader *reader, uint8_t *ptr);

/// Returns Hash struct containing hash
Hash finalize_shim(Hasher_shim *hasher);

/// Returns OutputReader struct for reading any number of bytes from hash.
OutputReader finalize_xof_shim(Hasher_shim *hasher);

void free_char_pointer(char *ptr_to_free);

void free_hasher(Hasher *ptr_to_free);

/// Returns nullptr if there are no errors
/// If error occurres, returns a char* with error message.
char *from_hex_shim(const char *hex_str, Hash *res);

/// Creates a new hasher for key derivation. If creation is successful, returns {Hasher_shim, nullptr}.
/// If error occurres, it will return Hasher_shim without a real hasher inside and error message as second field.
DerivedOut new_derive_key_shim(const char *context);

/// Creates a new hasher
Hasher_shim new_hasher();

/// Creates a new hasher fore keyed hash function
Hasher_shim new_keyed_shim(const uint8_t (*key)[KEY_LEN]);

/// Reset the Hasher contents
void reset_shim(Hasher_shim *hasher);

/// Returns char* if there are no errors
/// If error occurres, returns a nullptr.
char *to_hex_shim(const Hash *obj);

/// Give new input to hasher
/// Returns nullptr if everything is OK, returns an error message if input is nullptr.
/// Size parameter was added for compatibility with FunctionsHashing.h
char *update_shim(Hasher_shim *hasher, const char *input, uint32_t _size);

} // extern "C"
