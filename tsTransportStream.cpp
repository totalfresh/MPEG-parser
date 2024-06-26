#include "tsTransportStream.h"
#include <iostream>

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================

/// @brief Reset - reset all TS packet header fields
void xTS_PacketHeader::Reset()
{
    SB  = 0;
    E   = 0;
    S   = 0;
    T   = 0;
    PID = 0;
    TSC = 0;
    AFC = 0;
    CC  = 0;
}

/**
 * @brief Parse all TS packet header fields
 * @param Input is pointer to buffer containing TS packet
 * @return Number of parsed bytes (4 on success, -1 on failure) 
 */
int32_t xTS_PacketHeader::Parse(const uint8_t* Input)
{
    // obrócenie bitów przód - tył bo architektura tego wymaga
    uint32_t* H_ptr = (uint32_t*) Input;
    uint32_t  H_val = xSwapBytes32(*H_ptr);

    uint32_t E_m   = 0x00800000; 
    uint32_t S_m   = 0x00400000;
    uint32_t T_m   = 0x00200000; 
    uint32_t PID_m = 0x001FFF00;  
    uint32_t TSC_m = 0x000000C0;      
    uint32_t AFC_m = 0x00000030;     
    uint32_t CC_m  = 0x0000000F;      

    SB   = H_val >> 24;                 
    E    = (H_val & E_m)  >> 23;
    S    = (H_val & S_m)  >> 22;
    T    = (H_val & T_m)  >> 21;
    PID  = (H_val & PID_m)>>  8;
    TSC  = (H_val & TSC_m)>>  6;
    AFC  = (H_val & AFC_m)>>  4;
    CC   = H_val & CC_m;        

    return 4;
}

/// @brief Print all TS packet header fields
void xTS_PacketHeader::Print() const
{
    printf(" SB = %d", SB);
    printf(" E = %d", E);
    printf(" S = %d", S);
    printf(" T = %d", T);
    printf(" PID = %d", PID);
    printf(" TSC = %d", TSC);
    printf(" AFC = %d", AFC);
    printf(" CC = %d ", CC);
}

uint32_t xTS_PacketHeader::getPID() const {
  return PID;  // Upewnij się, że PID jest zmienną członkowską klasy xTS_PacketHeader
}

//=============================================================================================================================================================================
// xTS_AdaptationField
//=============================================================================================================================================================================

void xTS_AdaptationField::Reset()
{
    AFL = 0;
    DC = 0;
    RA = 0;
    SP = 0;
    PR = 0;
    OR = 0;
    SF = 0;
    TP = 0;
    EX = 0;
}

int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AFC)
{
    if (AFC == 2 || AFC == 3) {
        AFL = Input[0];
        if (AFL > 0) {
            uint8_t flags = Input[1];
            DC = (flags & 0x80) >> 7;
            RA = (flags & 0x40) >> 6;
            SP = (flags & 0x20) >> 5;
            PR = (flags & 0x10) >> 4;
            OR = (flags & 0x08) >> 3;
            SF = (flags & 0x04) >> 2;
            TP = (flags & 0x02) >> 1;
            EX = (flags & 0x01);
            return AFL + 1;  
        }
    }
    return 4;  
}

void xTS_AdaptationField::Print() const
{
    printf("\nAF: L = %d ", AFL);
    printf("DC = %d ", DC);
    printf("RA = %d ", RA);
    printf("SP = %d ", SP);
    printf("PR = %d ", PR);
    printf("OR = %d ", OR);
    printf("SF = %d ", SF);
    printf("TP = %d ", TP);
    printf("EX = %d ", EX);
}

//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================

xPES_PacketHeader::xPES_PacketHeader()
{
    Reset();
}

// Metoda Reset() resetuje wszystkie składowe nagłówka PES do ich wartości początkowych.
void xPES_PacketHeader::Reset()
{
    m_PacketStartCodePrefix = 0;
    m_StreamId = 0;
    m_PacketLength = 0;
}

// Metoda Parse() służy do analizy danych wejściowych i wyodrębnienia z nich informacji o nagłówku PES.
int32_t xPES_PacketHeader::Parse(const uint8_t* PacketBuffer){
        
        uint64_t* H_ptr = (uint64_t*) PacketBuffer;
        uint64_t  H_val = xSwapBytes64(*H_ptr);
        
        uint64_t PSCP_m = 0xFFFFFF0000000000; 
        uint64_t SId_m = 0x000000FF00000000; 
        uint64_t PL_m = 0x00000000FFFF0000; 

        m_PacketStartCodePrefix = (H_val & PSCP_m) >> 40;
        m_StreamId = (H_val & SId_m) >> 32;
        m_PacketLength = (H_val & PL_m) >> 16;
        
        if (m_StreamId != eStreamId::eStreamId_program_stream_map && 
            m_StreamId != eStreamId::eStreamId_padding_stream &&
            m_StreamId != eStreamId::eStreamId_private_stream_2 && 
            m_StreamId != eStreamId::eStreamId_ECM && 
            m_StreamId != eStreamId::eStreamId_EMM && 
            m_StreamId != eStreamId::eStreamId_program_stream_directory && 
            m_StreamId != eStreamId::eStreamId_DSMCC_stream && 
            m_StreamId != eStreamId::eStreamId_DSMCC_stream && 
            m_StreamId != eStreamId::eStreamId_ITUT_H222_1_type_E)
        {
            PacketBuffer+=8;
            return 9+PacketBuffer[0];
        }
        
        return 6;
}

// Metoda Print() wyświetla informacje o nagłówku PES.
void xPES_PacketHeader::Print() const
{
    printf("PES: PSCP=%X SID=%u L=%d\n",
           m_PacketStartCodePrefix, (int) m_StreamId, m_PacketLength);
}


//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================

// Metoda Init() inicjalizuje assembler PES, ustawiając identyfikator procesu (PID).
void xPES_Assembler::Init(int32_t PID)
{
    m_PID = PID;
    Helper = 0;
    xBufferReset();
    m_Started = false;
    m_LastContinuityCounter = -1;
}

// Resetuje stan assemblera PES oraz jego bufor.
void xPES_Assembler::Reset() {
    m_Buffer.clear();
    Helper = 0;
    m_LastContinuityCounter = -1;
    m_Started = false;
    m_PESH.Reset(); // Resetowanie nagłówka PES
}

// Metoda AbsorbPacket() przetwarza pojedynczy pakiet strumienia transportowego.
xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField) {
    if (PacketHeader->PID != m_PID) return eResult::UnexpectedPID;

    if (PacketHeader->getS() == 1) { // Sprawdzanie, czy jest to początek nowego pakietu PES
        Reset(); // Resetowanie stanu assemblera PES
    }

    // Sprawdzanie licznika ciągłości
    int8_t CC = PacketHeader->CC;
    if (m_Started && (m_LastContinuityCounter != -1) && (CC != ((m_LastContinuityCounter + 1) % 16))) {
        return eResult::StreamPacketLost;
    }

    m_LastContinuityCounter = CC;
    const uint8_t* Payload = TransportStreamPacket + xTS::TS_HeaderLength;
    int32_t PayloadSize = xTS::TS_PacketLength - xTS::TS_HeaderLength;
    if (PacketHeader->hasAdaptationField()) {
        Payload += 1 + AdaptationField->AFL;
        PayloadSize -= 1 + AdaptationField->AFL;
    }

    if (!m_Started) { 
        m_Started = true;
        Helper = m_PESH.Parse(Payload);
        m_Buffer.assign(Payload + Helper, Payload + PayloadSize);
        return eResult::AssemblingStarted;
    } else {
        m_Buffer.insert(m_Buffer.end(), Payload, Payload + PayloadSize);
        if (m_Buffer.size() >= m_PESH.getPacketLength() - Helper + 6)
            return eResult::AssemblingFinished;
        else
            return eResult::AssemblingContinue;
    }
}

void xPES_Assembler::SavePayloadToFile(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        std::cerr << "Cannot open file for writing.\n";
        return;
    }
    
    if (!m_Buffer.empty()) {
        fwrite(m_Buffer.data(), 1, m_Buffer.size(), file);
    }

    fclose(file);
}

// Metoda PrintPESH() wyświetla informacje o nagłówku PES.
void xPES_Assembler::PrintPESH() const
{
    m_PESH.Print();
}

// Zwraca wskaźnik do bufora z pakietami.
const uint8_t* xPES_Assembler::getPacket() const
{
    return m_Buffer.data();
}

// Zwraca liczbę bajtów w buforze.
int32_t xPES_Assembler::getNumPacketBytes() const
{
    return static_cast<int32_t>(m_Buffer.size());
}

// Resetuje bufor.
void xPES_Assembler::xBufferReset()
{
    m_Buffer.clear();
}

// Dodaje dane do bufora.
void xPES_Assembler::xBufferAppend(const uint8_t* Data, int32_t Size)
{
    m_Buffer.insert(m_Buffer.end(), Data, Data + Size);
}

// Konstruktor i destruktor klasy xPES_Assembler.
xPES_Assembler::xPES_Assembler() : m_PID(-1), m_LastContinuityCounter(-1), m_Started(false) {}
xPES_Assembler::~xPES_Assembler() {}
