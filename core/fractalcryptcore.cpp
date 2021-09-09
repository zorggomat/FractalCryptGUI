#include "fractalcryptcore.h"

const constexpr char FractalCryptCore::signature[8];

FractalCryptCore::FractalCryptCore()
{
    loop = new QEventLoop;
    progressDialog = new ProgressDialog();
    noizeCreator = new NoizeCreator();
    noizeCreator->setAutoDelete(false);
    aes = new AES();
    aes->setAutoDelete(false);

    QObject::connect(noizeCreator, &NoizeCreator::started, progressDialog, &ProgressDialog::start);
    QObject::connect(noizeCreator, &NoizeCreator::setMaximumValue, progressDialog, &ProgressDialog::setMaximum);
    QObject::connect(noizeCreator, &NoizeCreator::updateValue, progressDialog, &ProgressDialog::setValue);
    QObject::connect(noizeCreator, &NoizeCreator::finished, progressDialog, &ProgressDialog::quit);
    QObject::connect(noizeCreator, &NoizeCreator::finished, loop, &QEventLoop::quit);

    QObject::connect(aes, &AES::started, progressDialog, &ProgressDialog::start);
    QObject::connect(aes, &AES::setMaximumValue, progressDialog, &ProgressDialog::setMaximum);
    QObject::connect(aes, &AES::updateValue, progressDialog, &ProgressDialog::setValue);
    QObject::connect(aes, &AES::finished, progressDialog, &ProgressDialog::quit);
    QObject::connect(aes, &AES::finished, loop, &QEventLoop::quit);
}

FractalCryptCore::~FractalCryptCore()
{
    delete progressDialog;
    delete noizeCreator;
    delete aes;
}

FractalCryptCore& FractalCryptCore::Instance()
{
    static FractalCryptCore instance;
    return instance;
}

void FractalCryptCore::createNoize(QIODevice *iodevice, qint64 bytes)
{
    progressDialog->setText("Filling the container with random characters...");
    noizeCreator->setDevice(iodevice);
    noizeCreator->setNumberOfBytes(bytes);
    QThreadPool::globalInstance()->start(noizeCreator);
    loop->exec();
}

FractalCryptCore::StatusCode FractalCryptCore::encryptFile(QIODevice *file, QStringList passwords, const QVector<qint64> &offsets)
{
    aes->setIODevice(file);
    aes->setMode(Encrypt);
    for(int i = passwords.size()-1; i >= 0; --i)
    {
        progressDialog->setText("Encrypting layer " + QString::number(i+1));
        aes->setRange(offsets[i], file->size());
        aes->setPassword(passwords[i]);
        QThreadPool::globalInstance()->start(aes);
        loop->exec();
    }
    return OK;
}

FractalCryptCore::StatusCode FractalCryptCore::decryptFile(QIODevice *file, QStringList passwords, QVector<qint64> &offsets)
{
    qint64 pos = 0;
    qint64 fileSize = file->size();
    offsets.push_back(0);

    aes->setIODevice(file);
    aes->setMode(Decrypt);

    for(int i = 0; i < passwords.size(); ++i)
    {
        //Decrypt layer
        progressDialog->setText("Decrypting layer " + QString::number(i+1));
        aes->setRange(pos, fileSize);
        aes->setPassword(passwords.at(i).toUtf8());
        QThreadPool::globalInstance()->start(aes);
        loop->exec();

        //Read header
        qint64 layerSize;
        if(readHeader(file, pos, layerSize) != OK)
        {
            passwords.erase(passwords.begin()+i+1, passwords.end());
            encryptFile(file, passwords, offsets);
            return SignatureInvalid;
        }
        if(pos + layerSize + 16 > fileSize)
        {
            passwords.erase(passwords.begin()+i+1, passwords.end());
            encryptFile(file, passwords, offsets);
            return HeaderSizeInvalid;
        }

        //Next layer index
        if(layerSize % 16 != 0)
            layerSize += 16 - (layerSize % 16);
        pos += 16 + layerSize;
        offsets.push_back(pos);
    }

    return OK;
}

FractalCryptCore::StatusCode FractalCryptCore::resizeFile(QString path, QStringList passwords, qint64 newSize)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadWrite)) return ContainerUnavailable;
    qint64 currentSize = file.size();

    if(newSize == currentSize) return OK;

    QVector<qint64> offsets;
    StatusCode r = decryptFile(&file, passwords, offsets);
    if(r != OK) return r;

    if(offsets.back() > newSize)
    {
        offsets.pop_back();
        encryptFile(&file, passwords, offsets);
        return NewSizeTooSmall;
    }
    if(newSize < currentSize)
    {
        file.resize(newSize);
    }
    else
    {
        file.seek(currentSize);
        createNoize(&file, newSize - currentSize);
    }
    offsets.pop_back();
    return encryptFile(&file, passwords, offsets);;
}

FractalCryptCore::StatusCode FractalCryptCore::writeLayer(QString containerPath, QStringList files, QStringList directories, QStringList passwords, QString newPassword)
{
    //Open, decrypt and prepare container
    QFile container(containerPath);
    if(!container.open(QIODevice::ReadWrite)) return ContainerUnavailable;
    qint64 containerSize = container.size();

    QVector<qint64> offsets;
    StatusCode r = decryptFile(&container, passwords, offsets);
    if(r != OK) return r;
    container.close();

    //Write file
    qint64 size;
    if(!QuazipFunctions::writeZip(containerPath, offsets.back() + 16, files, directories, size))
    {
        offsets.pop_back();
        encryptFile(&container, passwords, offsets);
        return ZipError;
    }
    if(container.size() > containerSize)
    {
        container.resize(containerSize);
        container.open(QIODevice::ReadWrite);
        passwords.push_back("");
        encryptFile(&container, passwords, offsets);
        return NotEnoughSpace;
    }

    //Write header
    container.open(QIODevice::ReadWrite);
    writeHeader(&container, offsets.back(), size);

    //Encrypt and close container
    passwords.push_back(newPassword);
    return encryptFile(&container, passwords, offsets);
}

FractalCryptCore::StatusCode FractalCryptCore::readLayer(QString containerPath, QString filePath, QStringList passwords)
{
    //Open, decrypt and prepare container
    QFile container(containerPath);
    if(!container.open(QIODevice::ReadWrite)) return ContainerUnavailable;
    QVector<qint64> offsets;
    StatusCode r = decryptFile(&container, passwords, offsets);
    if (r != OK) return r;
    offsets.pop_back();

    //Get size
    qint64 layerSize;
    if(readHeader(&container, offsets.back(), layerSize) != OK)
    {
        encryptFile(&container, passwords, offsets);
        return SignatureInvalid;
    }
    if(offsets.back() + layerSize + 16 > container.size())
    {
        encryptFile(&container, passwords, offsets);
        return HeaderSizeInvalid;
    }

    //Read archive and encrypt container
    bool success = QuazipFunctions::readZip(container, offsets.back()+16, offsets.back()+16+layerSize, filePath);
    encryptFile(&container, passwords, offsets);
    return success ? OK : ZipError;
}

FractalCryptCore::StatusCode FractalCryptCore::removeLayer(QString containerPath, QStringList passwords)
{
    //Open, decrypt and prepare container
    QFile container(containerPath);
    if(!container.open(QIODevice::ReadWrite)) return ContainerUnavailable;
    QVector<qint64> offsets;
    StatusCode r = decryptFile(&container, passwords, offsets);
    if(r != OK) return r;
    container.seek(offsets.back());

    //Write noize and encrypt
    createNoize(&container, container.size() - offsets.back());
    passwords.push_back("");
    encryptFile(&container, passwords, offsets);
    return OK;
}

const QString& FractalCryptCore::getCodeDescription(StatusCode statusCode)
{
    static QMap<StatusCode, QString> map({
                                             {OK,                   "Operation successfully completed"},
                                             {ContainerUnavailable, "Unable to open the container"},
                                             {SignatureInvalid,     "Wrong password"},
                                             {HeaderSizeInvalid,    "The container is damaged or created incorrectly"},
                                             {NotEnoughSpace,       "Not enough space in the container"},
                                             {ZipError,             "Error processing zip archive"},
                                             {EncryptionError,      "Encryption error"},
                                             {NewSizeTooSmall,      "The new size is smaller than the size of the data in the container"}
                                         });
    return map[statusCode];
}

void FractalCryptCore::writeHeader(QIODevice *file, qint64 offset, quint64 size)
{
    file->seek(offset);
    file->write(signature, 8);
    char sizeBytes[8];
    for(int i = 0; i < 8; ++i)
        sizeBytes[i] = (size >> 8 * (7 - i));
    file->write(sizeBytes, 8);
}

FractalCryptCore::StatusCode FractalCryptCore::readHeader(QIODevice *file, qint64 offset, qint64 &size)
{
    file->seek(offset);
    quint8 headerSignature[8];
    quint8 headerSize[8];
    file->read((char*)headerSignature, 8);
    file->read((char*)headerSize, 8);
    if(strcmp((const char*)headerSignature, signature) != 0)
        return SignatureInvalid;
    size = 0;
    for(int j = 0; j < 8; ++j)
        size = (size << 8) + headerSize[j];
    return OK;
}
