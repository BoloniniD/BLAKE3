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

/// An incremental hash state that can accept any number of writes.
///
/// When the `traits-preview` Cargo feature is enabled, this type implements
/// several commonly used traits from the
/// [`digest`](https://crates.io/crates/digest) crate. However, those
/// traits aren't stable, and they're expected to change in incompatible ways
/// before that crate reaches 1.0. For that reason, this crate makes no SemVer
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


extern "C" {

char *blake3_apply_shim(const char *begin, uint32_t size, uint8_t *out_char_data);

void blake3_free_char_pointer(char *ptr_to_free);

void blake3_free_hasher(Hasher *ptr_to_free);

} // extern "C"
