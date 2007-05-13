/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "Base64Coder.h"
#include "Object.h"
#include "Runnable.h"
#include "Thread.h"
#include "System.h"
#include "TcpSocket.h"
#include "InternetAddress.h"
#include "SslSocket.h"
#include "MessageDigest.h"
#include "Crc16.h"
#include "AsymmetricKeyFactory.h"
#include "FileInputStream.h"
#include "DigitalEnvelope.h"
#include "DefaultBlockCipher.h"

using namespace std;
using namespace db::crypto;
using namespace db::io;
using namespace db::net;
using namespace db::rt;
using namespace db::util;

void runBase64Test()
{
	cout << "Running Base64 Test" << endl << endl;
	
	char data[] = {'a', 'b', 'c', 'd', 'e'};
	string encoded = Base64Coder::encode(data + 1, 4);
	cout << "encoded=" << encoded << endl;
	
   char* decoded;
	unsigned int length;
   Base64Coder::decode(encoded, &decoded, length);
	
	cout << "decoded bytes=" << length << endl;
   for(unsigned int i = 0; i < length; i++)
   {
      cout << "decoded[" << i << "]=" << decoded[i] << endl;
   }
	
	string encoded2 = Base64Coder::encode(decoded, 4);
	cout << "encoded again=" << encoded2 << endl;
	
   if(decoded != NULL)
   {
	   delete [] decoded;
   }
}

void runTimeTest()
{
   cout << "Running Time Test" << endl << endl;
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   cout << "Time start=" << start << endl;
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   cout << "Time end=" << end << endl;
}

class TestRunnable : public virtual Object, public Runnable
{
   virtual void run()
   {
      string name = Thread::currentThread()->getName();
      cout << name << ": This is a TestRunnable thread." << endl;
      
      if(name == "Thread 1")
      {
         cout << "Thread 1 Waiting for Thread 5..." << endl;
         
         lock();
         {
            lock();
            lock();
            lock();
            wait();
         }
         unlock();
         
         cout << "Thread 1 Finished." << endl;
      }
      else if(name == "Thread 3")
      {
         cout << "Thread 3 Waiting for Thread 5..." << endl;
         
         lock();
         lock();
         lock();
         lock();
         {
            wait();
         }
         unlock();
         
         cout << "Thread 3 Finished." << endl;
      }
      else if(name == "Thread 5")
      {
         cout << "Thread 5 waking up threads..." << endl;
         
         lock();
         lock();
         lock();
         lock();
         {
            notifyAll();
         }
         unlock();
      }
   }
};

void runThreadTest()
{
   cout << "Running Thread Test" << endl << endl;
   
   TestRunnable r1;
   Thread t1(&r1, "Thread 1");
   
   //TestRunnable r2;
   Thread t2(&r1, "Thread 2");
   
   //TestRunnable r3;
   Thread t3(&r1, "Thread 3");
   
   //TestRunnable r4;
   Thread t4(&r1, "Thread 4");
   
   //TestRunnable r5;
   Thread t5(&r1, "Thread 5");
   
   t1.start();
   t2.start();
   t3.start();
   t4.start();
   t5.start();
   
   t1.join();
   t2.join();
   t3.join();
   t4.join();
   t5.join();
}

void runLinuxSocketTest()
{
   cout << "Running Socket Test" << endl << endl;
   
   // create tcp socket
   TcpSocket socket;
   
   // create address
   //InternetAddress address("127.0.0.1", 1024);
   InternetAddress address("www.google.com", 80);
   cout << address.getAddress() << endl;
   
   // connect
   socket.connect(&address);
   
   char request[] =
      "GET / HTTP/1.0\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
   socket.send(request, sizeof(request));
   
   // set receive timeout (10 seconds = 10000 milliseconds)
   socket.setReceiveTimeout(10000);
   
   char response[2048];
   int numBytes = 0;
   string str = "";
   
   cout << endl << "DOING A PEEK!" << endl;
   
   numBytes = socket.getInputStream()->peek(response, 2048);
   if(numBytes != -1)
   {
      cout << "Peeked " << numBytes << " bytes." << endl;
      string peek = "";
      peek.append(response, numBytes);
      cout << "Peek bytes=" << peek << endl;
   }
   
   cout << endl << "DOING ACTUAL READ NOW!" << endl;
   
   while((numBytes = socket.getInputStream()->read(response, 2048)) != -1)
   {
      cout << "numBytes received: " << numBytes << endl;
      str.append(response, numBytes);
   }
   
//   char response[2048];
//   int numBytes = 0;
//   string str = "";
//   while((numBytes = socket.receive(response, 0, 2048)) != -1)
//   {
//      cout << "numBytes received: " << numBytes << endl;
//      str.append(response, numBytes);
//   }
   
   // close
   socket.close();
   
   cout << "Socket connection closed." << endl;
   cout << "Response:" << endl << str << endl;
   
   cout << endl << "Socket test complete." << endl;
}

void runWindowsSocketTest()
{
   // initialize winsock
#ifdef WIN32
   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
   {
      cout << "ERROR! Could not initialize winsock!" << endl;
   }
#endif
   
   // run linux socket test
   runLinuxSocketTest();
   
   // cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif
}

void runLinuxSslSocketTest()
{
   cout << "Running SSL Socket Test" << endl << endl;
   
   // openssl initialization code
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();
   
   // FIXME:
   // seed PRNG
   
   // create tcp socket
   TcpSocket socket;
   
   // create address
   InternetAddress address("127.0.0.1", 443);
   //InternetAddress address("127.0.0.1", 19020);
   //InternetAddress address("www.google.com", 80);
   cout << address.getAddress() << endl;
   
   // connect
   socket.connect(&address);
   
   // create an SSL context
   SslContext context;
   
   // create an SSL socket
   SslSocket sslSocket(&context, &socket, true, false);
   
   // set receive timeout (10 seconds = 10000 milliseconds)
   sslSocket.setReceiveTimeout(10000);
   
   // perform handshake (automatically happens, this call isn't necessary)
   //sslSocket.performHandshake();
   
   char request[] =
      "GET / HTTP/1.0\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
   sslSocket.send(request, sizeof(request));
   
   char response[2048];
   int numBytes = 0;
   string str = "";
   
   cout << endl << "DOING A PEEK!" << endl;
   
   numBytes = sslSocket.getInputStream()->peek(response, 2048);
   if(numBytes != -1)
   {
      cout << "Peeked " << numBytes << " bytes." << endl;
      string peek = "";
      peek.append(response, numBytes);
      cout << "Peek bytes=" << peek << endl;
   }
   
   cout << endl << "DOING ACTUAL READ NOW!" << endl;
   
   while((numBytes = sslSocket.getInputStream()->read(response, 2048)) != -1)
   {
      cout << "numBytes received: " << numBytes << endl;
      str.append(response, numBytes);
   }
   
   // close
   sslSocket.close();
   
   cout << "SSL Socket connection closed." << endl;
   cout << "Response:" << endl << str << endl;
   
   cout << endl << "SSL Socket test complete." << endl;
   
   // clean up SSL
   EVP_cleanup();
}

void runWindowsSslSocketTest()
{
// initialize winsock
#ifdef WIN32
   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
   {
      cout << "ERROR! Could not initialize winsock!" << endl;
   }
#endif
   
   // run linux ssl socket test
   runLinuxSslSocketTest();
   
// cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif   
}

void runLinuxServerSocketTest()
{
   cout << "Running Server Socket Test" << endl << endl;
   
   // create tcp socket
   TcpSocket socket;
   
   // bind and listen
   socket.bind(1024);
   socket.listen();
   
   // accept a connection
   Socket* worker = socket.accept(10);
   
   // set receive timeout (10 seconds = 10000 milliseconds)
   worker->setReceiveTimeout(10000);
   
   char request[2048];
   int numBytes = 0;
   string str = "";
   
   cout << endl << "DOING A PEEK!" << endl;
   
   numBytes = worker->getInputStream()->peek(request, 2048);
   if(numBytes != -1)
   {
      cout << "Peeked " << numBytes << " bytes." << endl;
      string peek = "";
      peek.append(request, numBytes);
      cout << "Peek bytes=" << peek << endl;
   }
   
   cout << endl << "DOING ACTUAL READ NOW!" << endl;
   
   while((numBytes = worker->getInputStream()->read(request, 2048)) != -1)
   {
      cout << "numBytes received: " << numBytes << endl;
      str.append(request, numBytes);
   }
   
   // close worker socket
   worker->close();
   delete worker;
   
   // close server socket
   socket.close();
   
   cout << "Server Socket connection closed." << endl;
   cout << "Request:" << endl << str << endl;
   
   cout << endl << "Socket test complete." << endl;
}

void runWindowsServerSocketTest()
{
// initialize winsock
#ifdef WIN32
   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
   {
      cout << "ERROR! Could not initialize winsock!" << endl;
   }
#endif
   
   // run linux server socket test
   runLinuxServerSocketTest();
   
// cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif   
}

void runMessageDigestTest()
{
   cout << "Running MessageDigest Test" << endl << endl;
   
   // correct values
   string correctMd5 = "78eebfd9d42958e3f31244f116ab7bbe";
   string correctSha1 = "5f24f4d6499fd2d44df6c6e94be8b14a796c071d";   
   
   MessageDigest testMd5("MD5");
   testMd5.update("THIS ");
   testMd5.update("IS A");
   testMd5.update(" MESSAGE");
   string digestMd5 = testMd5.getDigest();
   
   cout << "MD5 Digest=" << digestMd5 << endl;
   if(digestMd5 == correctMd5)
   {
      cout << "MD5 is correct!" << endl;
   }
   else
   {
      cout << "MD5 is incorrect!" << endl;
   }
   
   MessageDigest testSha1("SHA1");
   testSha1.update("THIS IS A MESSAGE");
   string digestSha1 = testSha1.getDigest();
   
   cout << "SHA-1 Digest=" << digestSha1 << endl;
   if(digestSha1 == correctSha1)
   {
      cout << "SHA-1 is correct!" << endl;
   }
   else
   {
      cout << "SHA-1 is incorrect!" << endl;
   }
   
   cout << "MessageDigest test complete." << endl << endl;
}

void runCrcTest()
{
   cout << "Running CRC Test" << endl << endl;
   
   unsigned int correctValue = 6013;
   
   Crc16 crc16;
   char b[] = {10, 20, 30, 40, 50, 60, 70, 80};
//   crc16.update(10);
//   crc16.update(20);
//   crc16.update(30);
//   crc16.update(40);
//   crc16.update(50);
//   crc16.update(60);
//   crc16.update(70);
//   crc16.update(80);
   crc16.update(b, 8);
   
   cout << "CRC-16=" << crc16.getChecksum() << endl;
   if(crc16.getChecksum() == correctValue)
   {
      cout << "CRC-16 is correct!" << endl;
   }
   else
   {
      cout << "CRC-16 is incorrect!" << endl;
   }
   
   cout << "CRC test complete." << endl << endl;
}

void runAsymmetricKeyLoadingTest()
{
   cout << "Running Asymmetric Key Loading Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   try
   {
      // read in PEM private key
      File file1("/work/src/dbcpp/dbcore/trunk/Debug/private.pem");
      FileInputStream fis1(&file1);
      
      string privatePem = "";
      
      char b[2048];
      int numBytes;
      while((numBytes = fis1.read(b, 2048)) != -1)
      {
         privatePem.append(b, numBytes);
      }
      
      // close stream
      fis1.close();
      
      cout << "Private Key PEM=" << endl << privatePem << endl;
      
      // read in PEM public key
      File file2("/work/src/dbcpp/dbcore/trunk/Debug/public.pem");
      FileInputStream fis2(&file2);
      
      string publicPem = "";
      
      while((numBytes = fis2.read(b, 2048)) != -1)
      {
         publicPem.append(b, numBytes);
      }
      
      // close stream
      fis2.close();
      
      cout << "Public Key PEM=" << endl << publicPem << endl;
           
      // get an asymmetric key factory
      AsymmetricKeyFactory factory;
      
      // load the private key
      PrivateKey* privateKey = factory.loadPrivateKeyFromPem(
         privatePem, "password");
      
      cout << "Private Key Algorithm=" << privateKey->getAlgorithm() << endl;
      
      // load the public key
      PublicKey* publicKey = factory.loadPublicKeyFromPem(publicPem);
      
      cout << "Public Key Algorithm=" << publicKey->getAlgorithm() << endl;
      
      // sign some data
      char data[] = {1,2,3,4,5,6,7,8};
      DigitalSignature* ds1 = privateKey->createSignature();
      ds1->update(data, 8);
      
      // get the signature
      char sig[ds1->getValueLength()];
      unsigned int length;
      ds1->getValue(sig, length);
      delete ds1;
      
      // verify the signature
      DigitalSignature* ds2 = publicKey->createSignature();
      ds2->update(data, 8);
      bool verified = ds2->verify(sig, length);
      delete ds2;
      
      if(verified)
      {
         cout << "Digital Signature Verified!" << endl;
      }
      else
      {
         cout << "Digital Signature NOT VERIFIED!" << endl;
      }
      
      string outPrivatePem =
         factory.writePrivateKeyToPem(privateKey, "password");
      string outPublicPem =
         factory.writePublicKeyToPem(publicKey);
      
      cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
      cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
      
      // delete the private key
      delete privateKey;
      
      // delete the public key
      delete publicKey;
   }
   catch(IOException &e)
   {
      cout << "IOException caught!" << endl;
      cout << e.getMessage() << endl;
      cout << e.getCode() << endl;
   }
   
   cout << "Asymmetric Key Loading test complete." << endl << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runDsaAsymmetricKeyCreationTest()
{
   cout << "Running DSA Asymmetric Key Creation Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   try
   {
      // get an asymmetric key factory
      AsymmetricKeyFactory factory;
      
      // create a new key pair
      PrivateKey* privateKey;
      PublicKey* publicKey;
      factory.createKeyPair("DSA", &privateKey, &publicKey);
      
      if(privateKey != NULL)
      {
         cout << "DSA Private Key created!" << endl;
      }
      else
      {
         cout << "DSA Private Key creation FAILED!" << endl;
      }
      
      if(publicKey != NULL)
      {
         cout << "DSA Public Key created!" << endl;
      }
      else
      {
         cout << "DSA Public Key creation FAILED!" << endl;
      }
      
      if(privateKey != NULL && publicKey != NULL)
      {
         cout << "Private Key Algorithm=" << privateKey->getAlgorithm() << endl;
         cout << "Public Key Algorithm=" << publicKey->getAlgorithm() << endl;
         
         // sign some data
         char data[] = {1,2,3,4,5,6,7,8};
         DigitalSignature* ds1 = privateKey->createSignature();
         ds1->update(data, 8);
         
         // get the signature
         char sig[ds1->getValueLength()];
         unsigned int length;
         ds1->getValue(sig, length);
         delete ds1;
         
         // verify the signature
         DigitalSignature* ds2 = publicKey->createSignature();
         ds2->update(data, 8);
         bool verified = ds2->verify(sig, length);
         delete ds2;
         
         if(verified)
         {
            cout << "Digital Signature Verified!" << endl;
         }
         else
         {
            cout << "Digital Signature NOT VERIFIED!" << endl;
         }
         
         string outPrivatePem =
            factory.writePrivateKeyToPem(privateKey, "password");
         string outPublicPem =
            factory.writePublicKeyToPem(publicKey);
         
         cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
         cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
      }
      
      // cleanup private key
      if(privateKey != NULL)
      {
         delete privateKey;
      }
      
      // cleanup public key
      if(publicKey != NULL)
      {
         delete publicKey;
      }
   }
   catch(Exception &e)
   {
      cout << "Exception caught!" << endl;
      cout << e.getMessage() << endl;
      cout << e.getCode() << endl;
   }
   
   cout << "DSA Asymmetric Key Creation test complete." << endl << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runRsaAsymmetricKeyCreationTest()
{
   cout << "Running RSA Asymmetric Key Creation Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   try
   {
      // get an asymmetric key factory
      AsymmetricKeyFactory factory;
      
      // create a new key pair
      PrivateKey* privateKey;
      PublicKey* publicKey;
      factory.createKeyPair("RSA", &privateKey, &publicKey);
      
      if(privateKey != NULL)
      {
         cout << "RSA Private Key created!" << endl;
      }
      else
      {
         cout << "RSA Private Key creation FAILED!" << endl;
      }
      
      if(publicKey != NULL)
      {
         cout << "RSA Public Key created!" << endl;
      }
      else
      {
         cout << "RSA Public Key creation FAILED!" << endl;
      }
      
      if(privateKey != NULL && publicKey != NULL)
      {
         cout << "Private Key Algorithm=" << privateKey->getAlgorithm() << endl;
         cout << "Public Key Algorithm=" << publicKey->getAlgorithm() << endl;
         
         // sign some data
         char data[] = {1,2,3,4,5,6,7,8};
         DigitalSignature* ds1 = privateKey->createSignature();
         ds1->update(data, 8);
         
         // get the signature
         char sig[ds1->getValueLength()];
         unsigned int length;
         ds1->getValue(sig, length);
         delete ds1;
         
         // verify the signature
         DigitalSignature* ds2 = publicKey->createSignature();
         ds2->update(data, 8);
         bool verified = ds2->verify(sig, length);
         delete ds2;
         
         if(verified)
         {
            cout << "Digital Signature Verified!" << endl;
         }
         else
         {
            cout << "Digital Signature NOT VERIFIED!" << endl;
         }
         
         string outPrivatePem =
            factory.writePrivateKeyToPem(privateKey, "password");
         string outPublicPem =
            factory.writePublicKeyToPem(publicKey);
         
         cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
         cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
      }
      
      // cleanup private key
      if(privateKey != NULL)
      {
         delete privateKey;
      }
      
      // cleanup public key
      if(publicKey != NULL)
      {
         delete publicKey;
      }
   }
   catch(Exception &e)
   {
      cout << "Exception caught!" << endl;
      cout << e.getMessage() << endl;
      cout << e.getCode() << endl;
   }
   
   cout << "RSA Asymmetric Key Creation test complete." << endl << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runEnvelopeTest(const std::string& algorithm)
{
   cout << "Running " << algorithm << " Envelope Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   try
   {
      // get an asymmetric key factory
      AsymmetricKeyFactory factory;
      
      // create a new key pair
      PrivateKey* privateKey;
      PublicKey* publicKey;
      factory.createKeyPair("RSA", &privateKey, &publicKey);
      
      if(privateKey != NULL && publicKey != NULL)
      {
         try
         {
            // create a secret message
            char message[] =
               "This is a confidential message. For British Eyes Only.";
            int length = strlen(message);
            
            string display1 = "";
            display1.append(message, length);
            cout << "Sending message '" << display1 << "'" << endl;
            cout << "Message Length=" << length << endl;
            
            // create an outgoing envelope
            SymmetricKey* secretKey;
            DigitalEnvelope* outEnv = publicKey->createEnvelope(
               "AES256", &secretKey);
            cout << "Created outgoing envelope..." << endl;
            
            // update the envelope
            char output[2048];
            int outLength;
            int totalOut = 0;
            outEnv->update(message, length, output, outLength);
            cout << "Updated outgoing envelope..." << endl;
            totalOut += outLength;
            
            // finish the envelope
            cout << "Output Length=" << outLength << endl;
            outEnv->finish(output + outLength, outLength);
            cout << "Finished sealing outgoing envelope..." << endl;
            totalOut += outLength;
            
            cout << "Total Output Length=" << totalOut << endl;
            
            // create an incoming envelope
            DigitalEnvelope* inEnv = privateKey->createEnvelope(secretKey);
            cout << "Created incoming envelope..." << endl;
            
            // update the envelope
            char input[2048];
            int inLength;
            int totalIn = 0;
            inEnv->update(output, totalOut, input, inLength);
            cout << "Updated incoming envelope..." << endl;
            totalIn += inLength;
            
            // finish the envelope
            cout << "Input Length=" << inLength << endl;
            inEnv->finish(input + inLength, inLength);
            cout << "Finished opening incoming envelope..." << endl;
            totalIn += inLength;
            
            cout << "Total Input Length=" << totalIn << endl;
            
            // create a string to display the received message
            string display2 = "";
            display2.append(input, totalIn);
            
            cout << "Received message '" << display2 << "'" << endl;
            
            // delete envelopes and key
            delete secretKey;
            delete outEnv;
            delete inEnv;
         }
         catch(IOException &e)
         {
            cout << "IOException caught in envelope method!" << endl;
            cout << "message: " << e.getMessage() << endl;
            cout << "code: " << e.getCode() << endl;
         }
      }
      
      // cleanup private key
      if(privateKey != NULL)
      {
         delete privateKey;
      }
      
      // cleanup public key
      if(publicKey != NULL)
      {
         delete publicKey;
      }
   }
   catch(Exception &e)
   {
      cout << "Exception caught!" << endl;
      cout << e.getMessage() << endl;
      cout << e.getCode() << endl;
   }
   
   cout << algorithm << " Envelope test complete." << endl << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runCipherTest(const string& algorithm)
{
   cout << "Running " << algorithm << " Cipher Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   try
   {
      // create a secret message
      char message[] = "I'll never teelllll!";
      int length = strlen(message);
      
      string display1 = "";
      display1.append(message, length);
      cout << "Encrypting message '" << display1 << "'" << endl;
      cout << "Message Length=" << length << endl;
      
      // get a default block cipher
      DefaultBlockCipher cipher;
      
      cout << "Starting encryption..." << endl;
      
      // generate a new key for the encryption
      SymmetricKey* key = NULL;
      cipher.startEncrypting(algorithm, &key);
      
      if(key != NULL)
      {
         // update encryption
         char output[2048];
         int outLength;
         int totalOut = 0;
         cipher.update(message, length, output, outLength);
         cout << "Updated encryption..." << endl;
         totalOut += outLength;
         
         // finish the envelope
         cout << "Output Length=" << outLength << endl;
         cipher.finish(output + outLength, outLength);
         cout << "Finished encryption..." << endl;
         totalOut += outLength;
         
         cout << "Total Output Length=" << totalOut << endl;
         
         cout << "Starting decryption..." << endl;
         cipher.startDecrypting(key);
         
         // update the decryption
         char input[2048];
         int inLength;
         int totalIn = 0;
         cipher.update(output, totalOut, input, inLength);
         cout << "Updated decryption..." << endl;
         totalIn += inLength;
         
         // finish the decryption
         cout << "Input Length=" << inLength << endl;
         cipher.finish(input + inLength, inLength);
         cout << "Finished decrypting..." << endl;
         totalIn += inLength;
         
         cout << "Total Input Length=" << totalIn << endl;
         
         // create a string to display the received message
         string display2 = "";
         display2.append(input, totalIn);
         
         cout << "Decrypted message '" << display2 << "'" << endl;
      }
      
      // cleanup key
      if(key != NULL)
      {
         delete key;
      }
   }
   catch(Exception &e)
   {
      cout << "Exception caught!" << endl;
      cout << e.getMessage() << endl;
      cout << e.getCode() << endl;
   }
   
   cout << algorithm << " Cipher test complete." << endl << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

int main()
{
   cout << "Tests starting..." << endl << endl;
   
   try
   {
      //runBase64Test();
      //runTimeTest();
      //runThreadTest();
      //runWindowsSocketTest();
      //runLinuxSocketTest();
      //runWindowsSslSocketTest();
      //runLinuxSslSocketTest();
      runWindowsServerSocketTest();
      //runLinuxServerSocketTest();
      //runMessageDigestTest();
      //runCrcTest();
      //runAsymmetricKeyLoadingTest();
      //runDsaAsymmetricKeyCreationTest();
      //runRsaAsymmetricKeyCreationTest();
      //runEnvelopeTest("DSA");
      //runEnvelopeTest("RSA");
      //runCipherTest("AES256");
   }
   catch(SocketException& e)
   {
      cout << "SocketException caught!" << endl;
      cout << "message: " << e.getMessage() << endl;
      cout << "code: " << e.getCode() << endl;
   }
   catch(UnsupportedAlgorithmException& e)
   {
      cout << "UnsupportedAlgorithmException caught!" << endl;
      cout << "message: " << e.getMessage() << endl;
      cout << "code: " << e.getCode() << endl;
   }
   catch(Exception &e)
   {
      cout << "Exception caught!" << endl;
      cout << "message: " << e.getMessage() << endl;
      cout << "code: " << e.getCode() << endl;
   }
   catch(...)
   {
      cout << "Exception caught!" << endl;
   }
   
   cout << endl << "Tests finished." << endl;
}
