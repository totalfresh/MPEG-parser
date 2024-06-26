# POLSKIE README POD ANG

# TS-PARSER

TS-PARSER is a tool for analyzing and parsing MPEG-TS transport streams. The program reads a `.ts` file, parses its headers, and then saves the PES (Packetized Elementary Stream) data for a specified PID to a separate file.

## Requirements

- CMake 3.10 or higher
- C++17 compatible compiler (e.g., GCC, Clang, MSVC)

## Build

To build the project, follow these steps:

1. Clone the repository:
    ```bash
    git clone https://github.com/totalfresh/Parser
    cd TS-PARSER
    ```

2. Create a build directory and navigate to it:
    ```bash
    mkdir build
    cd build
    ```

3. Run CMake to generate makefile/project files:
    ```bash
    cmake ..
    ```

4. Compile the project:
    ```bash
    make
    ```

## Usage

After compiling the program, you can run it to analyze a `.ts` file. The program will save the PES data for the specified PID (in this case, 136) to the file `PID136.mp2`.

### Example

1. Ensure that the file `example_new.ts` is in the same directory as the executable.

2. Run the program:
    ```bash
    ./TS-PARSER
    ```

### Output

The program will print information about the TS packets to the standard output, including headers and adaptation fields. The PES data for PID 136 will be saved in the file `PID136.mp2`.

## Project Structure

- `TS_parser.cpp`: The main source file containing the logic for parsing TS and PES.
- `tsCommon.h`: Contains helper functions for byte swapping.
- `tsTransportStream.h` and `tsTransportStream.cpp`: Contain definitions and implementations of classes for parsing TS and PES headers.

# TS-PARSER

TS-PARSER to narzędzie służące do analizy i parsowania strumieni transportowych MPEG-TS. Program odczytuje plik `.ts`, parsuje jego nagłówki, a następnie zapisuje dane PES (Packetized Elementary Stream) dla określonego PID do osobnego pliku.

## Wymagania

- CMake 3.10 lub nowszy
- Kompilator zgodny z C++17 (np. GCC, Clang, MSVC)

## Kompilacja

Aby skompilować projekt, wykonaj następujące kroki:

1. Sklonuj repozytorium:
    ```bash
    git clone https://github.com/totalfresh/Parser
    cd TS-PARSER
    ```

2. Utwórz katalog budowy i przejdź do niego:
    ```bash
    mkdir build
    cd build
    ```

3. Uruchom CMake, aby wygenerować pliki makefile/projektowe:
    ```bash
    cmake ..
    ```

4. Skompiluj projekt:
    ```bash
    make
    ```

## Użycie

Po skompilowaniu programu, możesz go uruchomić, aby przeanalizować plik `.ts`. Program zapisze dane PES dla określonego PID (w tym przypadku 136) do pliku `PID136.mp2`.

### Przykład

1. Upewnij się, że w tym samym katalogu co plik wykonywalny znajduje się plik `example_new.ts`.

2. Uruchom program:
    ```bash
    ./TS-PARSER
    ```

### Wyjście

Program wyświetli na standardowym wyjściu informacje o pakietach TS, w tym nagłówki i pola adaptacyjne. Dane PES dla PID 136 zostaną zapisane w pliku `PID136.mp2`.

## Struktura projektu

- `TS_parser.cpp`: Główny plik źródłowy zawierający logikę parsowania TS i PES.
- `tsCommon.h`: Zawiera pomocnicze funkcje do zamiany bajtów.
- `tsTransportStream.h` i `tsTransportStream.cpp`: Zawierają definicje i implementacje klas do parsowania nagłówków TS i PES.
