/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/Thread.h"
#include "db/crypto/AsymmetricKeyFactory.h"
#include "db/crypto/BigDecimal.h"
#include "db/crypto/BlockCipherInputStream.h"
#include "db/crypto/DigitalEnvelope.h"
#include "db/crypto/DigitalSignatureInputStream.h"
#include "db/crypto/DigitalSignatureOutputStream.h"
#include "db/crypto/DefaultBlockCipher.h"
#include "db/crypto/MessageDigest.h"
#include "db/io/File.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/io/FileList.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/io/OStreamOutputStream.h"

using namespace std;
using namespace db::test;
using namespace db::crypto;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

void runMessageDigestTest(TestRunner& tr)
{
   tr.group("MessageDigest");
   
   // correct values
   string correctMd5 = "78eebfd9d42958e3f31244f116ab7bbe";
   string correctSha1 = "5f24f4d6499fd2d44df6c6e94be8b14a796c071d";   
   
   tr.test("non-persistent");
   {
      MessageDigest testMd5("MD5", false);
      testMd5.update("THIS ");
      testMd5.update("IS A");
      testMd5.update(" MESSAGE");
      string digestMd5 = testMd5.getDigest();
      
      //cout << "MD5 Digest=" << digestMd5 << endl;
      assert(digestMd5 == correctMd5);
      
      MessageDigest testSha1("SHA1", false);
      testSha1.update("THIS IS A MESSAGE");
      string digestSha1 = testSha1.getDigest();
      
      //cout << "SHA-1 Digest=" << digestSha1 << endl;
      assert(digestSha1 == correctSha1);
   }
   tr.passIfNoException();
   
   tr.test("persistent");
   {
      string digestMd5;
      MessageDigest testMd5("MD5", true);
      testMd5.update("THIS ");
      digestMd5 = testMd5.getDigest();
      testMd5.update("IS A");
      digestMd5 = testMd5.getDigest();
      testMd5.update(" MESSAGE");
      digestMd5 = testMd5.getDigest();
      digestMd5 = testMd5.getDigest();
      
      //cout << "MD5 Digest=" << digestMd5 << endl;
      assert(digestMd5 == correctMd5);
      
      MessageDigest testSha1("SHA1", true);
      testSha1.update("THIS IS A MESSAGE");
      string digestSha1 = testSha1.getDigest();
      digestSha1 = testSha1.getDigest();
      
      //cout << "SHA-1 Digest=" << digestSha1 << endl;
      assert(digestSha1 == correctSha1);
      
      testSha1.reset();
      testSha1.update("THIS IS ");
      digestSha1 = testSha1.getDigest();
      testSha1.update("A MESSAGE");
      digestSha1 = testSha1.getDigest();
      digestSha1 = testSha1.getDigest();
      
      //cout << "SHA-1 Digest=" << digestSha1 << endl;
      assert(digestSha1 == correctSha1);
   }
   tr.passIfNoException();
   
   tr.ungroup();
}


void runCipherTest(TestRunner& tr, const char* algorithm)
{
   tr.group("Cipher");
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   tr.test(algorithm);
   {
      // create a secret message
      char message[] = "I'll never teelllll!";
      int length = strlen(message);
      
      // get a default block cipher
      DefaultBlockCipher cipher;
      
      // generate a new key and start encryption
      SymmetricKey key;
      cipher.startEncrypting(algorithm, &key);
      assertNoException();
      
      // update encryption
      char output[2048];
      int outLength;
      int totalOut = 0;
      cipher.update(message, length, output, outLength);
      totalOut += outLength;
      
      // finish encryption
      cipher.finish(output + outLength, outLength);
      totalOut += outLength;
      //printf("cipher out: %llu, out: %d\n", cipher.getTotalOutput(), totalOut);
      assert((int)cipher.getTotalOutput() == totalOut);
      
      // start decryption
      cipher.startDecrypting(&key);
      
      // update decryption
      char input[2048];
      int inLength;
      int totalIn = 0;
      cipher.update(output, totalOut, input, inLength);
      totalIn += inLength;
      
      // finish decryption
      cipher.finish(input + inLength, inLength);
      totalIn += inLength;
      //printf("cipher in: %llu, in: %d\n", cipher.getTotalOutput(), totalIn);
      assert((int)cipher.getTotalOutput() == totalIn);
      
      // check the decrypted message
      string result(input, totalIn);
      assert(strcmp(message, result.c_str()) == 0);
   }
   tr.passIfNoException();
   
   // do byte buffer test
   string alg = algorithm;
   alg.append("+ByteBuffer");
   tr.test(alg.c_str());
   {
      // create a secret message
      char message[] = "I'll never teelllll!";
      int length = strlen(message);
      
      // get a default block cipher
      DefaultBlockCipher cipher;
      
      // generate a new key and start encryption
      SymmetricKey key;
      cipher.startEncrypting(algorithm, &key);
      assertNoException();
      
      // update and finish encryption
      ByteBuffer output;
      cipher.update(message, length, &output, true);
      cipher.finish(&output, true);
      assert((int)cipher.getTotalInput() == length);
      assert((int)cipher.getTotalOutput() == output.length());
      
      // do decryption
      ByteBuffer input;
      cipher.startDecrypting(&key);
      cipher.update(output.data(), output.length(), &input, true);
      cipher.finish(&input, true);
      assert((int)cipher.getTotalInput() == output.length());
      assert((int)cipher.getTotalOutput() == input.length());
      
      // check the decrypted message
      string result(input.data(), input.length());
      assert(strcmp(message, result.c_str()) == 0);
   }
   tr.passIfNoException();
   
   alg = algorithm;
   alg.append("+BlockCipherInputStream");
   tr.test(alg.c_str());
   {
      // create a secret message
      char message[] = "I'll never teelllll!";
      ByteArrayInputStream bais(message, strlen(message));
      
      // get a default block cipher
      DefaultBlockCipher cipher;
      
      // generate a new key and start encryption
      SymmetricKey key;
      cipher.startEncrypting(algorithm, &key);
      assertNoException();
      
      // create encrypted data buffer
      ByteBuffer encrypted(200);
      
      // create stream to encrypt
      BlockCipherInputStream encryptStream(&cipher, false, &bais, false);
      char b[1024];
      int numBytes;
      while((numBytes = encryptStream.read(b, 1024)) > 0)
      {
         encrypted.put(b, numBytes, true);
      }
      encryptStream.close();
      assertNoException();
      
      // start decrypting
      cipher.startDecrypting(&key);
      
      // create decrypted data buffer
      ByteBuffer decrypted(200);
      
      // create stream to decrypt
      bais.setByteBuffer(&encrypted, false);
      BlockCipherInputStream decryptStream(&cipher, false, &bais, false);
      while((numBytes = decryptStream.read(b, 1024)) > 0)
      {
         decrypted.put(b, numBytes, true);
      }
      decryptStream.close();
      assertNoException();
      
      // assert data is the same
      string result(decrypted.data(), decrypted.length());
      assert(strcmp(message, result.c_str()) == 0);
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

void runAsymmetricKeyLoadingTest(TestRunner& tr)
{
   tr.test("Asymmetric Key Loading");
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("RSA", privateKey, publicKey);
   
   assert(!privateKey.isNull());
   assert(!publicKey.isNull());
   
   // write keys to PEMs
   string privatePem = factory.writePrivateKeyToPem(privateKey, "password");
   string publicPem = factory.writePublicKeyToPem(publicKey);   
   
   // cleanup keys
   privateKey.setNull();
   publicKey.setNull();
   
   // load the private key from PEM
   privateKey = factory.loadPrivateKeyFromPem(
      privatePem.c_str(), privatePem.length(), "password");
   
   // load the public key from PEM
   publicKey = factory.loadPublicKeyFromPem(
      publicPem.c_str(), publicPem.length());
   
   assert(!privateKey.isNull());
   assert(!publicKey.isNull());
   
   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature ds1(privateKey);
   ds1.update(data, 8);
   
   // get the signature
   char sig[ds1.getValueLength()];
   unsigned int length;
   ds1.getValue(sig, length);
   
   // verify the signature
   DigitalSignature ds2(publicKey);
   ds2.update(data, 8);
   bool verified = ds2.verify(sig, length);
   
   assert(verified);
   
   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, "password");
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);
   
   //cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
   //cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
   
   tr.passIfNoException();
}

void runDsaAsymmetricKeyCreationTest(TestRunner& tr)
{
   tr.test("DSA Asymmetric Key Creation");
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("DSA", privateKey, publicKey);
   
   assert(!privateKey.isNull());
   assert(!publicKey.isNull());
   
   assertStrCmp(privateKey->getAlgorithm(), "DSA");
   assertStrCmp(publicKey->getAlgorithm(), "DSA");
   
   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature ds1(privateKey);
   ds1.update(data, 8);
   
   // get the signature
   char sig[ds1.getValueLength()];
   unsigned int length;
   ds1.getValue(sig, length);
   
   // verify the signature
   DigitalSignature ds2(publicKey);
   ds2.update(data, 8);
   bool verified = ds2.verify(sig, length);
   
   assert(verified);
   
   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, "password");
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);
   
   //cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
   //cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
   
   tr.passIfNoException();
}

void runRsaAsymmetricKeyCreationTest(TestRunner& tr)
{
   tr.test("RSA Asymmetric Key Creation");
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("RSA", privateKey, publicKey);
   
   assert(!privateKey.isNull());
   assert(!publicKey.isNull());
   
   assertStrCmp(privateKey->getAlgorithm(), "RSA");
   assertStrCmp(publicKey->getAlgorithm(), "RSA");
   
   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature ds1(privateKey);
   ds1.update(data, 8);
   
   // get the signature
   char sig[ds1.getValueLength()];
   unsigned int length;
   ds1.getValue(sig, length);
   
   // verify the signature
   DigitalSignature ds2(publicKey);
   ds2.update(data, 8);
   bool verified = ds2.verify(sig, length);
   
   assert(verified);
   
   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, "password");
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);
   
   //cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
   //cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
   
   tr.passIfNoException();
}

void runDigitalSignatureInputStreamTest(TestRunner& tr)
{
   tr.test("DigitalSignatureInputStream");
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("RSA", privateKey, publicKey);
   
   assert(!privateKey.isNull());
   assert(!publicKey.isNull());
   
   assertStrCmp(privateKey->getAlgorithm(), "RSA");
   assertStrCmp(publicKey->getAlgorithm(), "RSA");
   
   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature* ds1 = new DigitalSignature(privateKey);
   
   char dummy[8];
   ByteArrayInputStream bais(data, 8);
   DigitalSignatureInputStream dsos1(ds1, true, &bais, false);
   dsos1.read(dummy, 8);
   
   // get the signature
   char sig[ds1->getValueLength()];
   unsigned int length;
   ds1->getValue(sig, length);
   
   // verify the signature
   DigitalSignature ds2(publicKey);
   bais.setByteArray(data, 8);
   DigitalSignatureInputStream dsos2(&ds2, false, &bais, false);
   dsos2.read(dummy, 8);
   bool verified = ds2.verify(sig, length);
   
   assert(verified);
   
   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, "password");
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);
   
   //cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
   //cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
   
   tr.passIfNoException();
}

void runDigitalSignatureOutputStreamTest(TestRunner& tr)
{
   tr.test("DigitalSignatureOutputStream");
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("RSA", privateKey, publicKey);
   
   assert(!privateKey.isNull());
   assert(!publicKey.isNull());
   
   assertStrCmp(privateKey->getAlgorithm(), "RSA");
   assertStrCmp(publicKey->getAlgorithm(), "RSA");
   
   // sign some data
   char data[] = {1,2,3,4,5,6,7,8};
   DigitalSignature ds1(privateKey);
   
   ostringstream oss;
   OStreamOutputStream osos(&oss);
   DigitalSignatureOutputStream dsos1(&ds1, false, &osos, false);
   dsos1.write(data, 8);
   
   // get the signature
   char sig[ds1.getValueLength()];
   unsigned int length;
   ds1.getValue(sig, length);
   
   // verify the signature
   DigitalSignature* ds2 = new DigitalSignature(publicKey);
   DigitalSignatureOutputStream dsos2(ds2, true, &osos, false);
   dsos2.write(data, 8);
   bool verified = ds2->verify(sig, length);
   
   assert(verified);
   
   string outPrivatePem =
      factory.writePrivateKeyToPem(privateKey, "password");
   string outPublicPem =
      factory.writePublicKeyToPem(publicKey);
   
   //cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
   //cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
   
   tr.passIfNoException();
}

void runEnvelopeTest(TestRunner& tr)
{
   tr.test("Envelope");
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   factory.createKeyPair("RSA", privateKey, publicKey);
   
   assert(!privateKey.isNull());
   assert(!publicKey.isNull());
   
   // create a secret message
   char message[] =
      "This is a confidential message. For British Eyes Only.";
   int length = strlen(message);
   
   string display1 = "";
   display1.append(message, length);
   //cout << "Sending message '" << display1 << "'" << endl;
   //cout << "Message Length=" << length << endl;
   
   // create an outgoing envelope
   SymmetricKey secretKey;
   DigitalEnvelope outEnv;
   outEnv.startSealing("AES256", publicKey, &secretKey);
   assertNoException();
   //cout << "Created outgoing envelope..." << endl;
   
   // update the envelope
   char output[2048];
   int outLength;
   int totalOut = 0;
   outEnv.update(message, length, output, outLength);
   //cout << "Updated outgoing envelope..." << endl;
   totalOut += outLength;
   
   // finish the envelope
   ///cout << "Output Length=" << outLength << endl;
   outEnv.finish(output + outLength, outLength);
   //cout << "Finished sealing outgoing envelope..." << endl;
   totalOut += outLength;
   
   //cout << "Total Output Length=" << totalOut << endl;
   
   // create an incoming envelope
   DigitalEnvelope inEnv;
   inEnv.startOpening(privateKey, &secretKey);
   assertNoException();
   //cout << "Created incoming envelope..." << endl;
   
   // update the envelope
   char input[2048];
   int inLength;
   int totalIn = 0;
   inEnv.update(output, totalOut, input, inLength);
   //cout << "Updated incoming envelope..." << endl;
   totalIn += inLength;
   
   // finish the envelope
   //cout << "Input Length=" << inLength << endl;
   inEnv.finish(input + inLength, inLength);
   //cout << "Finished opening incoming envelope..." << endl;
   totalIn += inLength;
   
   //cout << "Total Input Length=" << totalIn << endl;
   
   // create a string to display the received message
   string display2 = "";
   display2.append(input, totalIn);
   
   //cout << "Received message '" << display2 << "'" << endl;

   assert(display1 == display2);
   
   tr.passIfNoException();
}

void runBigIntegerTest(TestRunner& tr)
{
   tr.test("BigInteger");

   #define NSI(op, expectstr) \
   do { \
      BigInteger result = op; \
      assertStrCmp(result.toString().c_str(), expectstr); \
   } while(0)

   BigInteger number1 = 2;
   BigInteger number2 = 123456789;
   
   assert(number1 == 2);
   assert(number2 == 123456789);

   NSI(number1, "2");
   NSI(number2, "123456789");
   NSI(number1 + number2, "123456791");
   NSI(number1 - number2, "-123456787");
   NSI(number1 * number2, "246913578");
   NSI(number2 / number1, "61728394");
   NSI(number2 % number1, "1");
   NSI(number2.pow(number1), "15241578750190521");

   #undef NSI

   tr.passIfNoException();
}

void runBigDecimalTest(TestRunner& tr)
{
   tr.test("BigDecimal");
   
   #define BDCMP0(num, zerofill, expectedStr) \
   do { \
      BigDecimal result = num; \
      assertStrCmp(result.toString(zerofill).c_str(), expectedStr); \
   } while(0)

   BigDecimal number1 = 3.0;
   //BigDecimal number2 = 123456789.5;
   BigDecimal number2 = "123456789.53";
   //BigDecimal number2 = 1.234;
   //BigDecimal number2 = "1.23e-04";
   //BigDecimal number2 = "1234";
   
   // precision defaults to 10
   BDCMP0(number1, false, "3");
   BDCMP0(number2, false, "123456789.53");
   BDCMP0(number1 + number2, false, "123456792.53");
   BDCMP0(number1 - number2, false, "-123456786.53");
   BDCMP0(number1 * number2, false, "370370368.59");
   BDCMP0(number2 / number1, false, "41152263.1766666667");
   BDCMP0(number2 % number1, false, "0.53");

   #define BDCMP(num, precision, dir, zerofill, expectedStr) \
   do { \
      BigDecimal nr = num; \
      nr.setPrecision(precision, dir); \
      nr.round(); \
      assertStrCmp(nr.toString(zerofill).c_str(), expectedStr); \
   } while(0)
   
   // positive rounding
   {
      BigDecimal n = "129.54678010";
      BDCMP0(n, false, "129.54678010");
      
      BDCMP(n, 7, Up, false, "129.5467801");
      BDCMP(n, 6, Up, false, "129.546781");
      BDCMP(n, 5, Up, false, "129.54679");
      BDCMP(n, 4, Up, false, "129.5468");
      BDCMP(n, 3, Up, false, "129.547");
      BDCMP(n, 2, Up, false, "129.55");
      BDCMP(n, 1, Up, false, "129.6");
      BDCMP(n, 0, Up, false, "130");
   
      BDCMP(n, 7, HalfUp, false, "129.5467801");
      BDCMP(n, 6, HalfUp, false, "129.546780");
      BDCMP(n, 5, HalfUp, false, "129.54678");
      BDCMP(n, 4, HalfUp, false, "129.5468");
      BDCMP(n, 3, HalfUp, false, "129.547");
      BDCMP(n, 2, HalfUp, false, "129.55");
      BDCMP(n, 1, HalfUp, false, "129.5");
      BDCMP(n, 0, HalfUp, false, "130");
   
      BDCMP(n, 7, Down, false, "129.5467801");
      BDCMP(n, 6, Down, false, "129.546780");
      BDCMP(n, 5, Down, false, "129.54678");
      BDCMP(n, 4, Down, false, "129.5467");
      BDCMP(n, 3, Down, false, "129.546");
      BDCMP(n, 2, Down, false, "129.54");
      BDCMP(n, 1, Down, false, "129.5");
      BDCMP(n, 0, Down, false, "129");
   }
   
   // negative rounding
   {
      BigDecimal n = "-129.54678010";
      BDCMP0(n, false, "-129.54678010");
      
      printf("FIXME: rounding rules for negative numbers\n");
      /*
      BDCMP(n, 7, Up, false, "-129.5467801");
      BDCMP(n, 6, Up, false, "-129.546781");
      BDCMP(n, 5, Up, false, "-129.54679");
      BDCMP(n, 4, Up, false, "-129.5468");
      BDCMP(n, 3, Up, false, "-129.547");
      BDCMP(n, 2, Up, false, "-129.55");
      BDCMP(n, 1, Up, false, "-129.6");
      BDCMP(n, 0, Up, false, "-130");
   
      BDCMP(n, 7, HalfUp, false, "-129.5467801");
      BDCMP(n, 6, HalfUp, false, "-129.546780");
      BDCMP(n, 5, HalfUp, false, "-129.54678");
      BDCMP(n, 4, HalfUp, false, "-129.5468");
      BDCMP(n, 3, HalfUp, false, "-129.547");
      BDCMP(n, 2, HalfUp, false, "-129.55");
      BDCMP(n, 1, HalfUp, false, "-129.5");
      BDCMP(n, 0, HalfUp, false, "-130");
   
      BDCMP(n, 7, Down, false, "-129.5467801");
      BDCMP(n, 6, Down, false, "-129.546780");
      BDCMP(n, 5, Down, false, "-129.54678");
      BDCMP(n, 4, Down, false, "-129.5467");
      BDCMP(n, 3, Down, false, "-129.546");
      BDCMP(n, 2, Down, false, "-129.54");
      BDCMP(n, 1, Down, false, "-129.5");
      BDCMP(n, 0, Down, false, "-129");
      */
   }

   /*
   BigDecimal bd;
   
   for(int i = 7; i >= 0; i--)
   {
      bd = number3;
      bd.setPrecision(i, Up);
      bd.round();
      cout << "round " << i << " places, up=" << bd << endl;
   }
   
   for(int i = 7; i >= 0; i--)
   {
      bd = number3;
      bd.setPrecision(i, HalfUp);
      bd.round();
      cout << "round " << i << " places, half up=" << bd << endl;
   }
   
   for(int i = 7; i >= 0; i--)
   {
      bd = number3;
      bd.setPrecision(i, Down);
      bd.round();
      cout << "round " << i << " places, down=" << bd << endl;
   }
   */

   // FIXME: add more division tests
   
   {
      BigDecimal b1("100");
      BigDecimal b2("100.0");
      assert(b1 == b2);
   }
   
   {
      BigDecimal b1("-100");
      BigDecimal b2("-100.0");
      assert(b1 == b2);
   }
   
   {
      BigDecimal b1("25");
      BigDecimal b2("7");
      BDCMP(b1 / b2, 7, Up, false, "3.5714286");
   }
   
   {
      BigDecimal b1("0.80");
      BigDecimal b2("0.80");
      BDCMP(b1 / b2, 7, Up, false, "1");
   }
   
   {
      BigDecimal b1("8");
      BigDecimal b2("0.80000");
      BDCMP(b1 / b2, 7, Up, false, "10");
   }
   
   {
      BigDecimal b1("2");
      BigDecimal b2("0.500");
      BDCMP(b1 / b2, 7, Up, false, "4");
   }
   
   {
      BigDecimal d1;
      BigDecimal d2;
      BigDecimal max(100);
      BigDecimal inc("0.1");
      BigDecimal zero(0);
      printf("FIXME: rounding rules for negative numbers\n");
      printf("FIXME: fix comparison with printf half-even rounding\n");
      //for(d1 = -100; d1 <= max; d1 += inc)
      for(d1 = 0; d1 <= max; d1 += inc)
      {
         //for(d2 = -100; d2 <= max; d2 += inc)
         for(d2 = 0; d2 <= max; d2 += inc)
         {
            if(d2 != zero)
            {
               char res[100];
               sprintf(res, "%.7Lf", d1.getDouble() / d2.getDouble());
               /*
               BigDecimal d12;
               d12.setPrecision(7, HalfUp);
               d12 = d1 / d2;
               printf("%.7Lf / %.7Lf = %.7Lf (C:%.7Lf)\n",
                  d1.getDouble(), d2.getDouble(), d12.getDouble(),
                  d1.getDouble() / d2.getDouble());
               */
               //BDCMP(d1 / d2, 7, HalfUp, true, res);
            }
         }
      }
   }
   
   #undef BDCMP0
   #undef BDCMP

   tr.passIfNoException();
}

class DbCryptoTester : public db::test::Tester
{
public:
   DbCryptoTester()
   {
      setName("crypto");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runMessageDigestTest(tr);
      runCipherTest(tr, "AES256");
      runAsymmetricKeyLoadingTest(tr);
      runDsaAsymmetricKeyCreationTest(tr);
      runRsaAsymmetricKeyCreationTest(tr);
      runDigitalSignatureInputStreamTest(tr);
      runDigitalSignatureOutputStreamTest(tr);
      runEnvelopeTest(tr);
      runBigIntegerTest(tr);
      runBigDecimalTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbCryptoTester)
#endif
