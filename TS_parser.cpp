#include "tsCommon.h"
#include "tsTransportStream.h"
#include <iostream>
#include <cstdio>

//=============================================================================================================================================================================

int main(int argc, char *argv[], char *envp[])
{
    // Opening the input file
    FILE* fp = std::fopen("example_new.ts", "rb");
    if (!fp)
    {
        std::perror("File opening failed");
        return EXIT_FAILURE;
    }

    // Opening the output file
    FILE* fp_out = std::fopen("PID136.mp2", "wb");
    if (!fp_out)
    {
        std::perror("Output file opening failed");
        std::fclose(fp);
        return EXIT_FAILURE;
    }

    xTS_PacketHeader TS_PacketHeader;
    xTS_AdaptationField TS_AdaptationField;
    xPES_Assembler PES_Assembler;
    PES_Assembler.Init(136);

    const uint8_t BUFFER_SIZE = 188;
    uint8_t TS_PacketBuffer[BUFFER_SIZE];
    int32_t TS_PacketId = 0;

    // Looping through all packets until the end of the file
    while (true) {
        if (std::feof(fp)) {
            std::puts("End of file reached.");
            break;
        }

        size_t readBytes = fread(TS_PacketBuffer, 1, BUFFER_SIZE, fp);
        if (readBytes < BUFFER_SIZE) {
            std::puts("Error or end of file reached while reading a packet");
            break;
        }

        TS_PacketHeader.Reset();
        TS_PacketHeader.Parse(TS_PacketBuffer);
        printf("%010d ", TS_PacketId++);
        TS_PacketHeader.Print();

        if (TS_PacketHeader.getAFC() == 2 || TS_PacketHeader.getAFC() == 3) {
            TS_AdaptationField.Reset();
            TS_AdaptationField.Parse(TS_PacketBuffer + xTS::TS_HeaderLength, TS_PacketHeader.getAFC());
            TS_AdaptationField.Print();
        }

        if (TS_PacketHeader.getPID() == 136) {
            xPES_Assembler::eResult Result = PES_Assembler.AbsorbPacket(TS_PacketBuffer, &TS_PacketHeader, &TS_AdaptationField);
            switch(Result)
            {
                case xPES_Assembler::eResult::StreamPacketLost:
                    printf(" Packet lost\n");
                    break;
                case xPES_Assembler::eResult::AssemblingStarted:
                    printf(" Assembling started\n");
                    PES_Assembler.PrintPESH();
                    break;
                case xPES_Assembler::eResult::AssemblingContinue:
                    printf(" Assembling continues\n");
                    break;
                case xPES_Assembler::eResult::AssemblingFinished:
                    printf(" Assembling finished\n");
                    PES_Assembler.PrintPESH();
                    // Writing the assembled PES packet to file
                    fwrite(PES_Assembler.getPacket(), 1, PES_Assembler.getNumPacketBytes(), fp_out);
                    break;
                default:
                    break;
            }
        }

        printf("\n");
    }

    // Check for I/O errors and close the files
    if (std::ferror(fp))
        std::puts("I/O error when reading");
    else if (std::feof(fp))
        std::puts("End of file reached successfully");

    fclose(fp);  // Closing the input file
    fclose(fp_out);  // Closing the output file

    return EXIT_SUCCESS;
}

//=============================================================================================================================================================================
