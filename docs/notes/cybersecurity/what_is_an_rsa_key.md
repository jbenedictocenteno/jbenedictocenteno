# What is an RSA key?

## Introduction.

An RSA key looks mysterious until you print one. Then you see that it is only a few very big numbers.

The short version goes like this. Somebody picks two big prime numbers, tells nobody, and multiplies them together. That product is the **modulus**, `n` for short, and there is no need to hide it. Two more numbers complete the set, and people call them exponents: `e` is public and `d` is private.

- Your **public key** is `n` and `e`. You can give it to anyone.
- Your **private key** is `d`. It never leaves the place where you made it.

The two exponents are a pair. What you do with one, only the other can undo. That gives you two things:

- **Encryption.** Anyone encrypts a message with `e`. Only you can decrypt it, with `d`.
- **Signatures.** Only you can sign, with `d`. Anyone can check your signature, with `e`.

Why is it safe? Multiplying two primes takes microseconds. Going back from the product to the two primes is too hard for any computer, when the numbers are big enough. And without the two primes, nobody can work out `d`.

In the end, the whole idea is this. Anyone can check certain operations with your public key, but only your private key can do them. All of modern cryptography has that one idea as its foundation.

If what you want is to know what a key really is, this is the note to read first. [Signatures and identity](how_do_signatures_prove_identity.md) and [TLS](how_does_tls_work.md) build on it.

The commands in this note are from OpenSSL 3.

## How to make a key.

A key is made in five steps, and none of them is hard.

1. Start with two random primes, `p` and `q`. For RSA-2048 each of them is 1024 bits long, which is half of the key.
2. Multiply the two and call the product `n = p × q`. The "2048" in RSA-2048 is nothing more than the number of bits in `n`.
3. Now you need `λ = lcm(p − 1, q − 1)`, the smallest number that both `p − 1` and `q − 1` divide.
4. For `e`, take 65537, the same value that almost every key uses. It is prime, and it keeps the public operation cheap, at 17 multiplications.
5. Last comes `d`. It is the number that makes `e × d` leave a remainder of 1 when you divide by `λ`.

Step 5 is fast: the extended Euclidean algorithm finds `d` in microseconds. But look at what you need for it. Finding `d` takes `λ`, and finding `λ` takes `p` and `q`. That is the reason why nobody except the owner of the two primes can get `d`.

In step 3, a lot of books write `φ = (p − 1) × (q − 1)` where this note writes `λ`. Either one works. OpenSSL uses `λ`, and the `d` that comes out is smaller.

Now the four operations. They are all the same kind of calculation: raise to a power, then keep the remainder of the division by `n`.

| You want to | You compute | Who can do it |
|---|---|---|
| encrypt a message `m` | `c = mᵉ mod n` | anyone |
| decrypt `c` | `m = cᵈ mod n` | only you |
| sign a value `h` | `s = hᵈ mod n` | only you |
| check a signature `s` | `sᵉ mod n`, and see if you get `h` back | anyone |

Why does `d` undo `e`? Because `e × d = k·λ + 1`, for some whole number `k`. And any number raised to `k·λ + 1` gives the same number back, modulo `n`. That is Carmichael's theorem. There is nothing else behind it.

## Try it with small numbers.

You can check this key with a calculator.

| | |
|---|---|
| the primes | `p = 61`, `q = 53` |
| the modulus | `n = 3233` |
| `φ` and `λ` | `φ = 60 × 52 = 3120`, `λ = lcm(60, 52) = 780` |
| the exponents | `e = 17`; `d = 2753` from `φ` (`17 × 2753 = 15 × 3120 + 1`) or `d = 413` from `λ` (`17 × 413 = 9 × 780 + 1`) |
| encrypt 65 | `65¹⁷ mod 3233 = 2790` |
| decrypt 2790 | `2790⁴¹³ mod 3233 = 65` |
| sign 123 | `123⁴¹³ mod 3233 = 2746` |
| check 2746 | `2746¹⁷ mod 3233 = 123` |

Look at the two values of `d`. They differ by `3 × 780`, and that is a multiple of `λ`. So they give the same result for every number. A key this small needs a small `e`, so it takes 17. A real key takes 65537.

## Where do the primes come from?

Nobody has a list of 1024-bit primes. Your computer finds them by trying:

1. The random number generator makes an odd number of the right size.
2. A quick check throws the number away if a small prime divides it.
3. A stronger test, called **Miller–Rabin**, runs on the number many times. A number that is not prime fails each run at least 3 times in 4.
4. If the number fails, the computer goes back to step 1.

Near 2¹⁰²⁴, about 1 number in 710 is prime. So a key costs a few hundred tries. When `openssl genpkey` prints dots and plus signs, you are watching this search.

This is also the step where real products break. Two examples:

- **Two keys share a prime.** Then one calculation, `gcd(n₁, n₂)`, gives that prime to anyone, and both keys are broken. In 2012, a scan of the internet broke the keys of tens of thousands of network devices this way. Those devices made their keys at first boot. At that moment their random number generator had almost no randomness.
- **The two primes are too close.** Then both are close to `√n`, and a short search near `√n` finds them. The standards require the two primes to differ somewhere in their first 100 bits.

So remember this: the weakest part of an RSA key is the randomness that made it.

## How strong is each size?

| RSA key | As strong as a symmetric key of | As strong as a curve key of |
|---|---|---|
| RSA-2048 | 112 bits | 224 bits |
| RSA-3072 | 128 bits | 256 bits (P-256) |
| RSA-7680 | 192 bits | 384 bits (P-384) |
| RSA-15360 | 256 bits | 521 bits (P-521) |

These pairs come from NIST, in the document SP 800-57.

How far are attackers from 2048 bits? The biggest RSA modulus that anyone has factored in public has 829 bits. Its name is RSA-250. It took about 2700 core-years of computing, in 2020. A 2048-bit modulus is far out of reach of the same method.

And quantum computers? Shor's algorithm can factor fast on a big quantum computer. No quantum computer that exists is big enough. The post-quantum algorithms, ML-KEM and ML-DSA, are the standard answer to that risk.

## What is inside a key file?

Make a key and print it:

```bash
openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 -out key.pem
openssl pkey -in key.pem -noout -text
```

You get eight fields. Seven of them are long hex numbers, and one is `65537`.

| Field | Value | What it is for |
|---|---|---|
| `modulus` | `n` | public |
| `publicExponent` | `65537` | public |
| `privateExponent` | `d` | the private key itself |
| `prime1`, `prime2` | `p`, `q` | the secret that everything else comes from |
| `exponent1` | `d mod (p − 1)` | the CRT shortcut |
| `exponent2` | `d mod (q − 1)` | the CRT shortcut |
| `coefficient` | `q⁻¹ mod p` | the CRT shortcut |

The last three fields are there for speed. With `p` and `q`, you can do two small calculations and join the two results. The method is called CRT, for Chinese Remainder Theorem. You get the same answer, 3 to 4 times faster. Only the owner of the private key can take this shortcut, because it needs `p` and `q`.

And the public key? It is two of those eight fields and no more. To check that yourself, run `openssl pkey -in key.pem -pubout` and print what comes out: `Modulus` and `Exponent: 65537`.

And the file formats? They are wrappers around the same numbers.

- **DER** is the binary form. Think of a tree of records where every record carries a type, a length and a value; `openssl asn1parse` prints that tree for you. In DER, a 2048-bit private key takes 1218 bytes, and its public key takes 294.
- **PEM** is DER written as base64 text, between a `BEGIN` line and an `END` line. Text is easier to copy and to send. The `BEGIN` line tells you what is inside.

| The first line says | What is inside |
|---|---|
| `BEGIN PRIVATE KEY` | PKCS#8: the name of the algorithm, then the numbers. OpenSSL 3 writes this one. |
| `BEGIN RSA PRIVATE KEY` | PKCS#1: only the numbers. An earlier format. |
| `BEGIN ENCRYPTED PRIVATE KEY` | PKCS#8, encrypted with a passphrase. |
| `BEGIN PUBLIC KEY` | The modulus and the exponent. A certificate carries the same structure. |

## Nobody uses the plain formulas.

The formulas above have two problems when you use them directly.

- **Same input, same output.** A message always encrypts to the same number. So an attacker can encrypt guesses and compare the results.
- **Results combine.** `c₁ × c₂` decrypts to `m₁ × m₂`. So an attacker can build new valid messages without any key.

Real software fixes both with **padding**: extra bytes that go around the value before the calculation.

- **For encryption**, the padding has a name, OAEP, and it mixes random bytes into the value. The price is size: with RSA-2048, OAEP and SHA-256, 190 bytes is all that fits. That is why your data never goes through RSA. A small key goes through RSA, and a symmetric cipher uses that key on the data.
- **For signatures**, the message itself is never what gets signed. What gets signed is its **hash**, with padding around it. The padding comes in two kinds: PKCS#1 v1.5 uses fixed bytes, and PSS uses random ones.

[Signatures and identity](how_do_signatures_prove_identity.md) takes a real signature apart, byte by byte.

## And elliptic curves?

In other notes and tools you will find names like P-256, Ed25519 and X25519. Those are key pairs as well, only the maths is different. Here the private key is a random number, `k`. To get the public key, you take a fixed point on a curve and add it to itself `k` times. The point that you reach is the public key. Going from `k` to the point is fast. Going from the point back to `k` is too hard.

Curve keys are much smaller for the same strength, as the table above shows. They also need no search for primes. So new protocols prefer them.

RSA is still common where a small chip only checks signatures, like the boot ROM of a microcontroller. Checking an RSA signature is cheap: 17 multiplications.

## Easy to get wrong.

- **"RSA-2048 means a 2048-bit private key."** The 2048 bits belong to `n`, and `n` is the public part.
- **"Each key has its own secret `e`."** Nearly every key in the world has the same `e`, which is 65537. What a key keeps secret is `p`, `q` and whatever you compute from them.
- **"A bigger key fixes bad randomness."** No size helps here: two keys that share a prime are broken, however long they are.
- **"RSA encrypts my traffic."** With RSA-2048 and OAEP, 190 bytes is the most that RSA encrypts. The traffic goes through a symmetric cipher.
- **"A `.pem` file is a private key."** PEM is a text wrapper, and it can hold other things too. The `BEGIN` line is where you find out what a file holds.

## Related.

- [Signatures and identity](how_do_signatures_prove_identity.md) — what you do with `d`: you sign, and you prove that you hold the key
- [TLS](how_does_tls_work.md) — key pairs, certificates and a shared secret, together in one handshake
