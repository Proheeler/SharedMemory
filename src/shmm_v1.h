#ifndef SHMM_H
#define SHMM_H

#include <future>
#include <cstdint>
#include <memory>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdint>

struct my_array
{
    int* pointer;
    int size;
};



/*!
    \brief Класс для работы с разделяемой памятью

    Данный класс имеет только одну простую цель: Предоставить АПИ для работы с разделяемой памятью
*/

class BaseShmm
{
public:
    /*!
    Конструктор подключения к разделяемой памяти
    \param[in] shmid идентификатор сегмента памяти
    */
    explicit BaseShmm(int shmid);
    ~BaseShmm();
    /*!
    Функция создания сегмента памяти
    \param[in] memory_size Выделяемый размер памяти в байтах
    \param[in] blockSize_ Количество байтов, которые занимают данные
    */
    static int createSegment(int64_t memory_size,int64_t blockSize_);

    /*!
    Функция удаления сегмента памяти
    \param[in] shmid_ Идентификатор сегмента памяти
    */
    static int deleteSegment(int shmid_);

    /*!
    Функция чтения одного блока данных, блокирует блок памяти по данному индексу, необходимо потом использовать функцию Unlock
    \param[in] blockIndex Идентификатор блока сегмента памяти
    */
    void *readFromSegment(int64_t blockIndex);

    /*!
    Функция записи одного блока данных
    \param[in] blockIndex Идентификатор блока сегмента памяти
    \param[in] data Данные для записи (любой тип данных)
    */
    bool writeToSegment(int64_t blockIndex,void *data);

    /*!
    Функция разблокировки данных в памяти по индексу
    \param[in] blockIndex Идентификатор блока сегмента памяти
    */
    void unlock(int64_t blockIndex);

    /*!
    Функция для получения размера данных
    */
    int64_t getBlockSize() const;

    /*!
    Функция для получения рамера сегмента памяти
    */
    int64_t getMemorySize() const;

    /*!
    Функция для получения рамера сегмента памяти
    */
    int getMaxSize() const;

    /*!
    Функция для проверки существования сегмента памяти
    \param[in] shmid Идентификатор сегмента памяти
    */
    static bool checkShmid(int shmid);

    /*!
    Функция для обнуления блокировок в разделяемой памяти
    */
    void resetAtomics();

    int getShmid() const;


    /*!
    Функция для сегментов созданных этой библиотекой
    */
    static my_array findSegments();

private:
    int64_t memorySize;
    int64_t blockSize;
    int64_t dataShift;
    int sizeOfInt64;
    int maxSize;
    void* mem_ptr;
    int shmid;
};
#endif // SHMM_H
