<div align="center">
  <h1>
  <br>
  <img src="https://i.ibb.co/9gt2skv/logo.png" alt="Markdownify" width="200">
  <br>
  FractalCrypt
  <br>
  </h1>
  
  <h4>Free cryptoarchiver permitting any number of hidden volumes for deniable encryption</h4>

  <img src="https://forthebadge.com/images/badges/made-with-c-plus-plus.svg">
  <img src="https://forthebadge.com/images/badges/powered-by-qt.svg">
  <img src="https://forthebadge.com/images/badges/built-with-love.svg">
  <br>
  <img src="https://img.shields.io/travis/zorggish/FractalCryptGUI?style=flat-square">
  <img src="https://img.shields.io/github/stars/zorggish/FractalCryptGUI?style=flat-square">
  <img src="https://img.shields.io/codacy/grade/98d263fe8c7b4f879f2e365b94f80d75?style=flat-square">
  <img src="https://img.shields.io/github/license/zorggish/FractalCryptGUI?style=flat-square">
  <img src="https://img.shields.io/github/languages/code-size/zorggish/FractalCryptGUI?style=flat-square">
  <img src="https://img.shields.io/badge/contributions-welcome!-success?style=flat-square">
</div>
<br>

## What is deniable encryption, and what is it used for?
Deniable cryptography describes encryption methods, enabling to use [plausible denial](https://en.wikipedia.org/wiki/Plausible_deniability#Use_in_cryptography) of existing encrypted messages, which means there is no way to prove the existence of data in the plain text.

In many countries worldwide (for example, in the [UK](https://www.theverge.com/2017/5/17/15653786/rabbani-encryption-password-charged-terrorism-uk-airport) or the [US](https://www.theregister.com/2017/08/30/ex_cop_jailed_for_not_decrypting_data/)) you may be jailed or face legal issues for refusing to provide your password as required by law enforcement agencies, even if you are not suspected of any law violations.

Moreover, if a powerful group of people (government, a gang of bandits) have a suspicion that you store the necessary information on your laptop (confidential papers, bitcoin wallet), come to you for a search or robbery and find an encrypted file, they most likely will not analyze it for potential vulnerabilities, but [will simply torture you until you tell them the password](https://imgs.xkcd.com/comics/security.png).

FractalCrypt permits an encrypted file with an arbitrary number of keys. If any set of men ask you for a password, and you say that there is no information in the encrypted container or you have forgotten the password, it is far more likely that they will not believe you. Whereas, using FractalCrypt you can safely give away the keys to unclassified volumes, and there is no way to prove that there are actually more volumes than you have disclosed.

In addition, a file encrypted in such a way, unlike, let us say, a cryptocontainer on an unallocated drive space, can be transferred over the Internet, copied to a USB stick, etc.


## How does FractalCrypt work?
First, it creates a cryptocontainer of a user-specified size, filled with random characters. To create the first volume, the program archives the user-specified files to the beginning of the cryptocontainer and encrypts it using the first key. Thus, knowing the first key, you can access the first-layer files and the rest of the cryptocontainer.
[picture]

To create the second volume, the program will archive the files in the rest of the cryptocontainer and encrypt it with the second key; So, the first key is also needed to create or access data from the second volume.

The capability of plausible deniability is that the encrypted file is indistinguishable from noise; There is no way you can find out the amount of data stored in the cryptocontainer.

## How to use FractalCrypt?
In order to get N-layer files, the user must enter N passwords.

To add a new layer (also referred to as a volume) without risk (i.e., overwriting the noise), you should enter all the keys; otherwise, if you enter only a part of the keys, the new layer will be written over the encrypted data.

The user can also delete a layer, in this case it is overwritten with noise; All layers over it are removed as well, so only the top layer can be safely removed.

## Technical details 
AES-256 is used for encryption in CBC mode. SHA256 hash sum of password in utf8 is used as a key, md5 hash sum is used as an initialization vector (this may be insecure if the same password is used for several volumes).

Each layer, when decrypted, has the following header:

8 bytes - "fractal" c-string signature (null-terminated)

8 bytes - the length of the subsequent zip archive in big endian, aligned to 16 bytes

## Building
FractalCrypt uses the [Qt framework](https://account.qt.io/downloads)(downloading from the official site requires registration), the [OpenSSL library](https://www.openssl.org/) for encryption, and the [QuaZip library](http://quazip.sourceforge.net/) for Zip archives creation.

QuaZip is a wrapper over the Gilles Vollant's Minizip package (according to the license terms, we inform that the files of these libraries have been changed in our project) which uses the [zlib library](https://www.zlib.net/).

### Windows
For Windows, zlib comes with Qt.

You should individually download and install only OpenSSL, for instance, by following the [link](https://slproweb.com/products/Win32OpenSSL.html).

OpenSSL is supposed to be available at C:\OpenSSL-Win64, you can change this in your file FractalCrypt.pro. 

By default, Qt will try to link OpenSSL dynamically, in this case you may need the file C:\OpenSSL-Win64\bin\libcrypto-1_1-x64.dll in the directory with executable file.

### Linux
You should individually install the zlib and openssl libraries if they are not included in your distro.

On apt-based systems, this can be done with the following commands:
```
sudo apt-get install zlib1g-dev libssl-dev
```
#### How to build FractalCrypt without Qt Creator:
1. Install Qt libs
```
sudo apt-get install qt5-default qt5-qmake qtbase5-dev-tools qtchooser libqt5core5a qtbase-abi-5-12-8
```
2. Clone this repo
```
git clone https://github.com/zorggish/FractalCrypt.git
cd FractalCrypt
```
3. Build and run project
```
qmake -project
qmake FractalCrypt.pro
make
./FractalCrypt
```
