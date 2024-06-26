#pragma once
#include <iostream>
#include <cstdint>
#include "tsCommon.h"
#include <string>
#include <vector>

/*
MPEG-TS packet:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |                             Header                            | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   4 |                  Adaptation field + Payload                   | `
`     |                                                               | `
` 184 |                                                               | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `


MPEG-TS packet header:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |       SB      |E|S|T|           PID           |TSC|AFC|   CC  | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `

Sync byte                    (SB ) :  8 bits
Transport error indicator    (E  ) :  1 bit
Payload unit start indicator (S  ) :  1 bit
Transport priority           (T  ) :  1 bit
Packet Identifier            (PID) : 13 bits
Transport scrambling control (TSC) :  2 bits
Adaptation field control     (AFC) :  2 bits
Continuity counter           (CC ) :  4 bits
*/

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================

class xTS
{
public:
  static constexpr uint32_t TS_PacketLength  = 188;
  static constexpr uint32_t TS_HeaderLength  = 4;
  static constexpr uint32_t PES_HeaderLength = 6;
  static constexpr uint32_t BaseClockFrequency_Hz         =    90000; //Hz
  static constexpr uint32_t ExtendedClockFrequency_Hz     = 27000000; //Hz
  static constexpr uint32_t BaseClockFrequency_kHz        =       90; //kHz
  static constexpr uint32_t ExtendedClockFrequency_kHz    =    27000; //kHz
  static constexpr uint32_t BaseToExtendedClockMultiplier =      300;
};

class xTS_PacketHeader
{
public:
  enum class ePID : uint16_t
  {
    PAT  = 0x0000,
    CAT  = 0x0001,
    TSDT = 0x0002,
    IPMT = 0x0003,
    NIT  = 0x0010,
    SDT  = 0x0011, 
    NuLL = 0x1FFF,
  };

  uint32_t SB;
  uint32_t E;
  uint32_t S;
  uint32_t T;
  uint32_t PID;
  uint32_t TSC;
  uint32_t AFC;
  uint32_t CC;

public:
  void Reset();
  int32_t Parse(const uint8_t* Input);
  void Print() const;

  uint32_t getAFC() const { return AFC; }
  uint32_t getS() const { return S; }
  uint32_t getPID() const;

  uint8_t getSyncByte() const { return static_cast<uint8_t>(SB); }
  bool hasAdaptationField() const { return AFC == 2 || AFC == 3; }
  bool hasPayload() const { return AFC == 1 || AFC == 3; }
};

//=============================================================================================================================================================================
// xTS_AdaptationField
//=============================================================================================================================================================================

class xTS_AdaptationField
{
public:
  uint8_t AFL; // Adaptation Field Length
  uint8_t DC;  // Discontinuity Indicator
  uint8_t RA;  // Random Access Indicator
  uint8_t SP;  // Elementary Stream Priority Indicator
  uint8_t PR;  // Program Clock Reference Flag
  uint8_t OR;  // Original Program Clock Reference Flag
  uint8_t SF;  // Splicing Point Flag
  uint8_t TP;  // Transport Private Data Flag
  uint8_t EX;  // Adaptation Field Extension Flag

  xTS_AdaptationField() { Reset(); }

  void Reset();

  int32_t Parse(const uint8_t* Input, uint8_t AFC);
  void Print() const;
};

//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================

// Klasa xPES_PacketHeader zarządza nagłówkiem pakietu PES, który rozpoczyna każdy pakiet PES w strumieniu MPEG-TS.
class xPES_PacketHeader
{
public:
    enum eStreamId : uint8_t
    {
        eStreamId_program_stream_map = 0xBC,
        eStreamId_padding_stream = 0xBE,
        eStreamId_private_stream_2 = 0xBF,
        eStreamId_ECM = 0xF0,
        eStreamId_EMM = 0xF1,
        eStreamId_program_stream_directory = 0xFF,
        eStreamId_DSMCC_stream = 0xF2,
        eStreamId_ITUT_H222_1_type_E = 0xF8,
    };

    xPES_PacketHeader();
    void Reset();                        // Resetuje stan nagłówka do stanu początkowego.
    int32_t Parse(const uint8_t* Input); // Analizuje nagłówek PES z bufora danych.
    void Print() const;                  // Wyświetla informacje o nagłówku PES.
    uint32_t getPacketStartCodePrefix() const; // Zwraca prefix startowy pakietu.
    uint8_t getStreamId() const;               // Zwraca identyfikator strumienia.
    uint16_t xPES_PacketHeader::getPacketLength() const 
    {
    return m_PacketLength;
    }


protected:
    uint32_t m_PacketStartCodePrefix;          // Prefix startowy, który oznacza początek pakietu PES.
    uint8_t m_StreamId;                        // Identyfikator strumienia określa typ danych w pakiecie PES.
    uint16_t m_PacketLength;                   // Długość danych w pakiecie PES, nie licząc tego nagłówka.
    uint8_t m_HeaderLength;
};

//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================

class xPES_Assembler
{
public:
    enum class eResult : int32_t
    {
        UnexpectedPID = 1, 
        StreamPacketLost, 
        AssemblingStarted, 
        AssemblingContinue,
        AssemblingFinished, 
    };

    xPES_Assembler();
    ~xPES_Assembler();
    void Init(int32_t PID);
    // Absorbcja pakietu TS i przetwarzanie go.
    eResult AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField);
    void PrintPESH() const;            // Wyświetla informacje o skompilowanym nagłówku PES.
    const uint8_t* getPacket() const;  // Zwraca skompilowany pakiet PES.
    int32_t getNumPacketBytes() const; // Zwraca liczbę bajtów w skompilowanym pakiecie PES.
    void Reset();                      // Resetuje stan assemblera.
    void SavePayloadToFile(const char* filename);
protected:
    void xBufferReset();               // Resetuje bufor danych.
    void xBufferAppend(const uint8_t* Data, int32_t Size); // Dodaje dane do bufora.

    int32_t m_PID;                     // PID strumienia, który jest przetwarzany.
    std::vector<uint8_t> m_Buffer;     // Bufor na dane pakietu PES.
    xPES_PacketHeader m_PESH;          // Nagłówek pakietu PES.
    int8_t m_LastContinuityCounter;    // Ostatnia wartość licznika ciągłości.
    bool m_Started;                    // Czy składanie pakietu zostało rozpoczęte.
    int32_t Helper;                    // Zmienna pomocnicza do nagłówka PES
};
