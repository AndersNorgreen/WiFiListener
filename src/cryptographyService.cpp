#include <cryptographyService.h>

String CryptographyService::hash(const char* input) {
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mdInfo, 1);  // 1 = HMAC - kan sættes til nul, hvis MQTT TLS virker
  
  int ret = mbedtls_pkcs5_pbkdf2_hmac(
    &ctx,
    (const unsigned char*)input,
    strlen(input),
    salt,
    sizeof(salt),
    iterations,
    KEY_LEN,
    key);
  
  mbedtls_md_free(&ctx);
  
  if(ret != 0)
    return "Hashing failed";

  return stringify(key, KEY_LEN);
}

bool CryptographyService::verifyHash(const char* input, const String& match) {
  String hashedInput = hash(input);
  return hashedInput == match;
}

String CryptographyService::encrypt(const String& plaintext) {
  unsigned char iv[IV_LEN];
  for (int i = 0; i < IV_LEN; ++i) iv[i] = random(0, 256);

  const uint8_t* input = (const uint8_t*)plaintext.c_str();
  size_t inputLen = plaintext.length();

  unsigned char ciphertext[inputLen];
  unsigned char tag[TAG_LEN];

  mbedtls_gcm_context ctx;
  mbedtls_gcm_init(&ctx);
  mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, KEY_LEN * 8);

  int ret = mbedtls_gcm_crypt_and_tag(
    &ctx, MBEDTLS_GCM_ENCRYPT, inputLen,
    iv, IV_LEN,
    NULL, 0,                 // No AAD
    input,
    ciphertext,
    TAG_LEN, tag
  );

  mbedtls_gcm_free(&ctx);
  if (ret != 0) return "Encryption failed";

  // Concatenate IV + ciphertext + tag
  size_t totalLen = IV_LEN + inputLen + TAG_LEN;
  unsigned char final[totalLen];
  memcpy(final, iv, IV_LEN);
  memcpy(final + IV_LEN, ciphertext, inputLen);
  memcpy(final + IV_LEN + inputLen, tag, TAG_LEN);

  // Base64 encode
  size_t b64Len;
  unsigned char b64Out[(totalLen * 4 / 3) + 4];
  mbedtls_base64_encode(b64Out, sizeof(b64Out), &b64Len, final, totalLen);
  b64Out[b64Len] = '\0';

  return String((char*)b64Out);
}

String CryptographyService::decrypt(const String& base64Ciphertext) {
  size_t decodedLen;
  unsigned char decoded[base64Ciphertext.length() * 3 / 4 + 1];

  int ret = mbedtls_base64_decode(
    decoded, sizeof(decoded), &decodedLen,
    (const unsigned char*)base64Ciphertext.c_str(),
    base64Ciphertext.length()
  );

  if (ret != 0 || decodedLen < (IV_LEN + TAG_LEN)) return "Base64 decode failed";

  unsigned char* iv = decoded;
  unsigned char* ciphertext = decoded + IV_LEN;
  size_t cipherLen = decodedLen - IV_LEN - TAG_LEN;
  unsigned char* tag = decoded + decodedLen - TAG_LEN;

  unsigned char output[cipherLen];

  mbedtls_gcm_context ctx;
  mbedtls_gcm_init(&ctx);
  mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, KEY_LEN * 8);

  ret = mbedtls_gcm_auth_decrypt(
    &ctx, cipherLen,
    iv, IV_LEN,
    NULL, 0,
    tag, TAG_LEN,
    ciphertext,
    output
  );

  mbedtls_gcm_free(&ctx);
  if (ret != 0) return "Decryption failed";

  output[cipherLen] = '\0';  // ensure null-terminated
  return String((char*)output);
}

void CryptographyService::generateIV(unsigned char* iv) {
  for (size_t i = 0; i < IV_LEN; ++i) {
    iv[i] = random(0, 256);
  }
}

bool CryptographyService::pkcs7Unpad(unsigned char* data, size_t& len) {
  if (len == 0) return false;

  size_t pad = data[len - 1];
  if (pad == 0 || pad > 16 || pad > len) {
    Serial.printf("Invalid pad value: %u (len = %u)\n", pad, len);
    return false;
  }

  for (size_t i = 0; i < pad; ++i) {
    if (data[len - 1 - i] != pad) {
      Serial.printf("Padding mismatch at byte %u: got %u, expected %u\n", len - 1 - i, data[len - 1 - i], pad);
      return false;
    }
  }

  len -= pad;
  return true;
}

size_t CryptographyService::pkcs7Pad(const unsigned char* input, size_t inputLen, unsigned char* output) {
  size_t padding = 16 - (inputLen % 16);
  memcpy(output, input, inputLen);
  for (size_t i = 0; i < padding; ++i) {
    output[inputLen + i] = padding;
  }
  return inputLen + padding;
}

void CryptographyService::deriveKey(const char* password) {
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);

  int ret = mbedtls_md_setup(&ctx, mdInfo, 1);  // 1 = HMAC
  if (ret != 0) {
      Serial.println("md_setup failed");
      return;
  }

  ret = mbedtls_pkcs5_pbkdf2_hmac(
      &ctx,
      (const unsigned char*)password, strlen(password),
      salt, sizeof(salt),
      iterations,
      KEY_LEN,
      key
  );

  mbedtls_md_free(&ctx);

  if (ret != 0) {
      Serial.println("Key derivation failed");
  }
}

String CryptographyService::stringify(const unsigned char* input, size_t inputLen) {
  String stringifiedInput;
  char buf[3];
  for (size_t i = 0; i < inputLen; i++) {
    sprintf(buf, "%02x", input[i]);
    stringifiedInput += buf;
  }

  return stringifiedInput;
}