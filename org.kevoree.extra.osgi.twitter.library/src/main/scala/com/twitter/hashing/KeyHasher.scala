package com.twitter.hashing

import java.nio.{ByteBuffer, ByteOrder}
import java.security.MessageDigest
import scala.collection.mutable

/**
 * Hashes a key into a 32-bit or 64-bit number (depending on the algorithm).
 *
 */
trait KeyHasher {
  def hashKey(key: Array[Byte]): Long
}

/**
 * Commonly used key hashing algorithms.
 */
object KeyHasher {
  /**
   * FNV fast hashing algorithm in 32 bits.
   * @see http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash
   */
  val FNV1_32 = new KeyHasher {
    def hashKey(key: Array[Byte]): Long = {
      val PRIME: Int = 16777619
      var i = 0
      val len = key.length
      var rv: Long = 0x811c9dc5L
      while (i < len) {
        rv = (rv * PRIME) ^ (key(i) & 0xff)
        i += 1
      }
      rv & 0xffffffffL
    }

    override def toString() = "FNV1_32"
  }

  /**
   * FNV fast hashing algorithm in 32 bits, variant with operations reversed.
   * @see http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash
   */
  val FNV1A_32 = new KeyHasher {
    def hashKey(key: Array[Byte]): Long = {
      val PRIME: Int = 16777619
      var i = 0
      val len = key.length
      var rv: Long = 0x811c9dc5L
      while (i < len) {
        rv = (rv ^ (key(i) & 0xff)) * PRIME
        i += 1
      }
      rv & 0xffffffffL
    }

    override def toString() = "FNV1A_32"
  }

  /**
   * FNV fast hashing algorithm in 64 bits.
   * @see http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash
   */
  val FNV1_64 = new KeyHasher {
    def hashKey(key: Array[Byte]): Long = {
      val PRIME: Long = 1099511628211L
      var i = 0
      val len = key.length
      var rv: Long = 0xcbf29ce484222325L
      while (i < len) {
        rv = (rv * PRIME) ^ (key(i) & 0xff)
        i += 1
      }
      rv & 0xffffffffL
    }

    override def toString() = "FNV1_64"
  }

  /**
   * FNV fast hashing algorithm in 64 bits, variant with operations reversed.
   * @see http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash
   */
  val FNV1A_64 = new KeyHasher {
    def hashKey(key: Array[Byte]): Long = {
      val PRIME: Long = 1099511628211L
      var i = 0
      val len = key.length
      var rv: Long = 0xcbf29ce484222325L
      while (i < len) {
        rv = (rv ^ (key(i) & 0xff)) * PRIME
        i += 1
      }
      rv & 0xffffffffL
    }

    override def toString() = "FNV1A_64"
  }

  /**
   * Ketama's default hash algorithm: the first 4 bytes of the MD5 as a little-endian int.
   * Wow, really? Who thought that was a good way to do it? :(
   */
  val KETAMA = new KeyHasher {
    def hashKey(key: Array[Byte]): Long = {
      val hasher = MessageDigest.getInstance("MD5")
      hasher.update(key)
      val buffer = ByteBuffer.wrap(hasher.digest)
      buffer.order(ByteOrder.LITTLE_ENDIAN)
      buffer.getInt.toLong & 0xffffffffL
    }

    override def toString() = "Ketama"
  }

  /**
   * The default memcache hash algorithm is the ITU-T variant of CRC-32.
   */
  val CRC32_ITU = new KeyHasher {
    def hashKey(key: Array[Byte]): Long = {
      var i = 0
      val len = key.length
      var rv: Long = 0xffffffffL
      while (i < len) {
        rv = rv ^ (key(i) & 0xff)
        var j = 0
        while (j < 8) {
          if ((rv & 1) != 0) {
            rv = (rv >> 1) ^ 0xedb88320L
          } else {
            rv >>= 1
          }
          j += 1
        }
        i += 1
      }
      (rv ^ 0xffffffffL) & 0xffffffffL
    }

    override def toString() = "CRC32_ITU"
  }

  /**
   * Paul Hsieh's hash function.
   * http://www.azillionmonkeys.com/qed/hash.html
   */
  val HSIEH = new KeyHasher {
    override def hashKey(key: Array[Byte]): Long = {
      var hash: Int = 0

      if (key.isEmpty)
        return 0

      for (i <- 0 until key.length / 4) {
        val b0 = key(i*4)
        val b1 = key(i*4 + 1)
        val b2 = key(i*4 + 2)
        val b3 = key(i*4 + 3)
        val s0 = (b1 << 8) | b0
        val s1 = (b3 << 8) | b2

        hash += s0
        val tmp = (s1 << 11) ^ hash
        hash = (hash << 16) ^ tmp
        hash += hash >>> 11
      }

      val rem = key.length % 4
      val offset = key.length - rem
      rem match {
        case 3 =>
          val b0 = key(offset)
          val b1 = key(offset + 1)
          val b2 = key(offset + 2)
          val s0 = b1 << 8 | b0
          hash += s0
          hash ^= hash << 16
          hash ^= b2 << 18
          hash += hash >>> 11
        case 2 =>
          val b0 = key(offset)
          val b1 = key(offset + 1)
          val s0 = b1 << 8 | b0
          hash += s0
          hash ^= hash << 11
          hash += hash >>> 17
        case 1 =>
          val b0 = key(offset)
          hash += b0
          hash ^= hash << 10
          hash += hash >>> 1
        case 0 => ()
      }

      hash ^= hash << 3
      hash += hash >>> 5
      hash ^= hash << 4
      hash += hash >>> 17
      hash ^= hash << 25
      hash += hash >>> 6

      hash & 0xffffffffL
    }

    override def toString() = "Hsieh"
  }


  /**
  * Jenkins Hash Function
  * http://en.wikipedia.org/wiki/Jenkins_hash_function
  */
  val JENKINS = new KeyHasher {
    private val MAX_VALUE = 0xFFFFFFFFL
    override def hashKey(key: Array[Byte]): Long = {
      var hash: Long = 0

      for (i <- 0 until key.size) {
        hash = (hash + key(i)) & MAX_VALUE
        hash = (hash + (hash << 10)) & MAX_VALUE
        hash = (hash + (hash >> 6)) & MAX_VALUE
      }

      hash = (hash + (hash << 3)) & MAX_VALUE
      hash = (hash ^ (hash >> 11)) & MAX_VALUE
      (hash + (hash << 15)) & MAX_VALUE
    }
  }

  /**
   * Return one of the key hashing algorithms by name. This is used to configure a memcache
   * client from a config file.
   */
  def byName(name: String): KeyHasher = {
    name match {
      case "fnv" => FNV1_32
      case "fnv1" => FNV1_32
      case "fnv1-32" => FNV1_32
      case "fnv1a-32" => FNV1A_32
      case "fnv1-64" => FNV1_64
      case "fnv1a-64" => FNV1A_64
      case "ketama" => KETAMA
      case "crc32-itu" => CRC32_ITU
      case "hsieh" => HSIEH
      case _ => throw new NoSuchElementException(name)
    }
  }
}
