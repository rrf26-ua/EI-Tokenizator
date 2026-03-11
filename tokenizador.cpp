// RAÚL RUIZ FLORES, 50593447W

#include "tokenizador.h"

#include <fstream>
#include <iostream>
#include <cctype>
#include <sys/stat.h>
#include <dirent.h>

namespace {
static bool is_digit(unsigned char c) {
    return (c >= '0' && c <= '9');
}
static inline void StripCR(std::string& s) {
    if (!s.empty() && s.back() == '\r') s.pop_back();
}
}

std::ostream& operator<<(std::ostream& os, const Tokenizador& t) {
    os << "DELIMITADORES: " << t.delimiters
       << " TRATA CASOS ESPECIALES: " << t.casosEspeciales
       << " PASAR A MINUSCULAS Y SIN ACENTOS: " << t.pasarAminuscSinAcentos;
    return os;
}

Tokenizador::Tokenizador(const std::string& delimitadoresPalabra,
                         const bool& kcasosEspeciales,
                         const bool& minuscSinAcentos)
    : delimiters(FiltrarRepetidos(delimitadoresPalabra)),
      casosEspeciales(kcasosEspeciales),
      pasarAminuscSinAcentos(minuscSinAcentos) {
    RebuildDelimiterTable();
}

Tokenizador::Tokenizador(const Tokenizador& other)
    : delimiters(other.delimiters),
      casosEspeciales(other.casosEspeciales),
      pasarAminuscSinAcentos(other.pasarAminuscSinAcentos) {
    for (int i = 0; i < 256; ++i) delimTable[i] = other.delimTable[i];
}

Tokenizador::Tokenizador()
    : delimiters(",:.-/+*\\ '\"{}[]()<>\xA1!\xBF?&#=\t@"),
      casosEspeciales(true),
      pasarAminuscSinAcentos(false) {
    delimiters = FiltrarRepetidos(delimiters);
    RebuildDelimiterTable();
}

Tokenizador::~Tokenizador() {
    delimiters.clear();
    RebuildDelimiterTable();
}

Tokenizador& Tokenizador::operator=(const Tokenizador& other) {
    if (this != &other) {
        delimiters = other.delimiters;
        casosEspeciales = other.casosEspeciales;
        pasarAminuscSinAcentos = other.pasarAminuscSinAcentos;
        for (int i = 0; i < 256; ++i) delimTable[i] = other.delimTable[i];
    }
    return *this;
}

void Tokenizador::RebuildDelimiterTable() {
    for (int i = 0; i < 256; ++i) delimTable[i] = false;
    for (std::size_t i = 0; i < delimiters.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(delimiters[i]);
        delimTable[c] = true;
    }
}

std::string Tokenizador::FiltrarRepetidos(const std::string& s) {
    bool seen[256];
    for (int i = 0; i < 256; ++i) seen[i] = false;

    std::string out;
    out.reserve(s.size());

    for (std::size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (!seen[c]) {
            seen[c] = true;
            out.push_back(static_cast<char>(c));
        }
    }
    return out;
}

bool Tokenizador::EsBlanco(unsigned char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

bool Tokenizador::EsDelimitador(unsigned char c) const {
    return delimTable[c];
}

bool Tokenizador::EsSeparadorCasosEspeciales(unsigned char c) const {

    if (EsBlanco(c)) return true;
    return EsDelimitador(c);
}

unsigned char Tokenizador::NormalizaCharISO8859_1(unsigned char c) {
    // Conversión ISO-8859-1 a letras ASCII en minúscula (sin acentos).
    // Solo se mapean los caracteres relevantes
    switch (c) {
        // Variantes de A / a
        case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5:
        case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5:
            return static_cast<unsigned char>('a');

        // Variantes de E / e
        case 0xC8: case 0xC9: case 0xCA: case 0xCB:
        case 0xE8: case 0xE9: case 0xEA: case 0xEB:
            return static_cast<unsigned char>('e');

        // Variantes de I / i
        case 0xCC: case 0xCD: case 0xCE: case 0xCF:
        case 0xEC: case 0xED: case 0xEE: case 0xEF:
            return static_cast<unsigned char>('i');

        // Variantes de O / o
        case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6:
        case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6:
            return static_cast<unsigned char>('o');

        // Variantes de U / u
        case 0xD9: case 0xDA: case 0xDB: case 0xDC:
        case 0xF9: case 0xFA: case 0xFB: case 0xFC:
            return static_cast<unsigned char>('u');

        // Ñ / ñ -> keep ñ
        case 0xD1: // Ñ
        case 0xF1: // ñ
            return static_cast<unsigned char>(0xF1); // ñ

        // Ç / ç  -> keep c
        case 0xC7: case 0xE7:
            return static_cast<unsigned char>('c');

        // Y / y
        case 0xDD: case 0xFD: case 0xFF:
            return static_cast<unsigned char>('y');

        default:
            break;
    }

    // ASCII letters to lowercase
    if (c >= 'A' && c <= 'Z') return static_cast<unsigned char>(c - 'A' + 'a');
    return c;
}

std::string Tokenizador::NormalizaCadena(const std::string& s) const {
    if (!pasarAminuscSinAcentos) return s;
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        out.push_back(static_cast<char>(NormalizaCharISO8859_1(c)));
    }
    return out;
}

void Tokenizador::DelimitadoresPalabra(const std::string& nuevoDelimiters) {
    delimiters = FiltrarRepetidos(nuevoDelimiters);
    RebuildDelimiterTable();
}

void Tokenizador::AnyadirDelimitadoresPalabra(const std::string& nuevoDelimiters) {
    bool present[256];
    for (int i = 0; i < 256; ++i) present[i] = false;
    for (std::size_t i = 0; i < delimiters.size(); ++i) {
        present[static_cast<unsigned char>(delimiters[i])] = true;
    }

    for (std::size_t i = 0; i < nuevoDelimiters.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(nuevoDelimiters[i]);
        if (!present[c]) {
            present[c] = true;
            delimiters.push_back(static_cast<char>(c));
        }
    }
    RebuildDelimiterTable();
}

std::string Tokenizador::DelimitadoresPalabra() const {
    return delimiters;
}

void Tokenizador::CasosEspeciales(const bool& nuevoCasosEspeciales) {
    casosEspeciales = nuevoCasosEspeciales;
}

bool Tokenizador::CasosEspeciales() {
    return casosEspeciales;
}

void Tokenizador::PasarAminuscSinAcentos(const bool& nuevoPasarAminuscSinAcentos) {
    pasarAminuscSinAcentos = nuevoPasarAminuscSinAcentos;
}

bool Tokenizador::PasarAminuscSinAcentos() {
    return pasarAminuscSinAcentos;
}

bool Tokenizador::ParseURL(const std::string& s, std::size_t pos, std::size_t& end) const {
    // Delimitadores internos permitidos en URLs (se permiten dentro del token URL).
    // "_:/.?&-=#@".
    const std::string allowed = "_:/.?&-=#@";
    auto is_allowed = [&](unsigned char c) -> bool {
        for (std::size_t i = 0; i < allowed.size(); ++i) {
            if (static_cast<unsigned char>(allowed[i]) == c) return true;
        }
        return false;
    };

    const char* p1 = "http:";
    const char* p2 = "https:";
    const char* p3 = "ftp:";

    std::size_t len = 0;
    if (s.compare(pos, 5, p1) == 0) len = 5;
    else if (s.compare(pos, 6, p2) == 0) len = 6;
    else if (s.compare(pos, 4, p3) == 0) len = 4;
    else return false;

    if (pos + len >= s.size()) return false;
    unsigned char next = static_cast<unsigned char>(s[pos + len]);
    if (EsBlanco(next)) return false;
    if (EsDelimitador(next) && !is_allowed(next)) return false;

    std::size_t k = pos;
    while (k < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[k]);
        if (EsBlanco(c)) break;
        if (EsDelimitador(c) && !is_allowed(c)) break;
        ++k;
    }

    end = k;
    return (end > pos);
}

bool Tokenizador::ParseNumero(const std::string& s,
                              std::size_t pos,
                              std::size_t& end,
                              std::string& outToken,
                              std::string& pendingSymbol) const
{
    outToken.clear();
    pendingSymbol.clear();
    end = pos;

    auto is_digit = [](unsigned char ch) { return ch >= '0' && ch <= '9'; };
    auto peek = [&](std::size_t p) -> unsigned char {
        if (p >= s.size()) return 0;
        return static_cast<unsigned char>(s[p]);
    };

    std::size_t k = pos;

    // Puede empezar por dígito, o por '.'/',' SOLO si después viene un dígito.
    unsigned char c0 = peek(k);
    if (c0 == '.' || c0 == ',') {
        unsigned char nx = peek(k + 1);
        if (!is_digit(nx)) return false;
        outToken.push_back('0');
        outToken.push_back(static_cast<char>(c0));
        ++k; // dejamos el dígito para el bucle
    } else if (is_digit(c0)) {
        // ok
    } else {
        return false;
    }

    bool sawAnyDigit = false;

    while (k < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[k]);

        // Cortan por blanco
        if (EsBlanco(c)) break;
        
        if (EsDelimitador(c) && c != '.' && c != ',') break;

        // Dígitos
        if (is_digit(c)) {
            outToken.push_back(static_cast<char>(c));
            sawAnyDigit = true;
            ++k;
            continue;
        }

        // '.' o ',' dentro del número:
        if (c == '.' || c == ',') {
            unsigned char nx = peek(k + 1);

            // Si '.'/',' seguido de blanco/fin -> termina n
            if (nx == 0 || EsBlanco(nx)) break;

            // CLAVE: si '.'/',' va seguido de NO dígito => NO aceptar el prefijo como número
            
            if (!is_digit(nx)) return false;

            
            if (outToken.empty() || !is_digit(static_cast<unsigned char>(outToken.back())))
                return false;

            outToken.push_back(static_cast<char>(c));
            ++k;
            continue;
        }

        // % o $ solo cuentan como "final de número" si van seguidos de blanco/fin.
        if (c == '%' || c == '$') {
            unsigned char nx = peek(k + 1);
            if (nx == 0 || EsBlanco(nx)) {
                pendingSymbol.assign(1, static_cast<char>(c)); // se tokeniza después
                ++k; // consumimos el símbolo
                break;
            }
            // Si NO va seguido de blanco, NO es número según heurística
            return false;
        }

        // Cualquier otra cosa (letra, /, +, -, E, etc.) => NO es número
        return false;
    }

    if (!sawAnyDigit) return false;

    end = k;                 // end = posición donde termina el parseo
    return (end > pos);
}

bool Tokenizador::ParseEmail(const std::string& s, std::size_t pos, std::size_t& end) const {
    if (!EsDelimitador(static_cast<unsigned char>('@'))) return false;

    std::size_t k = pos;

    // Parte local del e-mail: no puede contener delimitadores ni blancos.
    if (k >= s.size()) return false;
    if (EsBlanco(static_cast<unsigned char>(s[k])) || EsDelimitador(static_cast<unsigned char>(s[k]))) return false;

    while (k < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[k]);
        if (EsBlanco(c) || EsDelimitador(c)) break;
        ++k;
    }

    if (k >= s.size()) return false;
    if (static_cast<unsigned char>(s[k]) != '@') return false;

    // Después de '@' debe venir un carácter que no sea delimitador ni blanco.
    if (k + 1 >= s.size()) return false;
    unsigned char after = static_cast<unsigned char>(s[k + 1]);
    if (EsBlanco(after) || EsDelimitador(after)) return false;

    // Parte dominio: puede incluir .-_ solo si van rodeados de no-delimitadores y sin blancos.
    std::size_t j = k + 1;
    bool sawSecondAt = false;

    while (j < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[j]);
        if (EsBlanco(c)) break;

        if (c == '@' && EsDelimitador(c)) {
            sawSecondAt = true;
            break;
        }

        if (EsDelimitador(c)) {
            if (c == '.' || c == '-' || c == '_') {
                // Solo permitido si está rodeado por no-delimitadores (y sin blancos).
                if (j == k + 1) break; // first after '@' cannot be .-_ (needs left non-delim)
                if (j + 1 >= s.size()) break;

                unsigned char left = static_cast<unsigned char>(s[j - 1]);
                unsigned char right = static_cast<unsigned char>(s[j + 1]);

                if (EsBlanco(left) || EsDelimitador(left)) break;
                if (EsBlanco(right) || EsDelimitador(right)) break;

                ++j;
                continue;
            }
            break;
        }

        ++j;
    }

    if (sawSecondAt) return false;

    // Debe haber al menos un carácter en la parte de dominio.
    if (j == k + 1) return false;

    end = j;
    return true;
}

bool Tokenizador::ParseAcronimo(const std::string& s,
                                std::size_t pos,
                                std::size_t& end,
                                std::string& outToken) const {
    outToken.clear();

    if (!EsDelimitador(static_cast<unsigned char>('.'))) return false;

    std::size_t k = pos;
    bool sawDot = false;
    unsigned char prev = 0;

    while (k < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[k]);
        if (EsBlanco(c)) break;

        if (EsDelimitador(c) && c != '.') break;

        if (c == '.') {
            if (prev == '.') {
                // Ends on multiple consecutive dots.
                break;
            }
            sawDot = true;
        }

        prev = c;
        ++k;
    }

    if (!sawDot) return false;

    std::string candidate = s.substr(pos, k - pos);

    // Eliminar puntos al inicio/final.
    while (!candidate.empty() && candidate[0] == '.') candidate.erase(candidate.begin());
    while (!candidate.empty() && candidate[candidate.size() - 1] == '.') candidate.erase(candidate.end() - 1);

    if (candidate.empty()) return false;

    // Tras recortar, debe quedar al menos un punto.
    bool hasDot = false;
    for (std::size_t i = 0; i < candidate.size(); ++i) {
        if (candidate[i] == '.') { hasDot = true; break; }
    }
    if (!hasDot) return false;

    // Se rechaza si contiene "..".
    for (std::size_t i = 1; i < candidate.size(); ++i) {
        if (candidate[i] == '.' && candidate[i - 1] == '.') return false;
    }

    outToken = candidate;

    end = k;
    return true;
}

bool Tokenizador::ParseMultipalabraGuiones(const std::string& s,
                                           std::size_t pos,
                                           std::size_t& end,
                                           std::string& outToken) const {
    outToken.clear();

    if (!EsDelimitador(static_cast<unsigned char>('-'))) return false;

    std::size_t k = pos;
    bool sawHyphenInside = false;

    while (k < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[k]);
        if (EsBlanco(c)) break;

        if (EsDelimitador(c)) {
            if (c == '-') {
                // Solo permitido si está rodeado por no-separadores (ni blanco ni delimitador).
                if (k == pos) break;
                if (k + 1 >= s.size()) break;

                unsigned char left = static_cast<unsigned char>(s[k - 1]);
                unsigned char right = static_cast<unsigned char>(s[k + 1]);

                if (EsBlanco(left) || EsDelimitador(left)) break;
                if (EsBlanco(right) || EsDelimitador(right)) break;

                sawHyphenInside = true;
                outToken.push_back('-');
                ++k;
                continue;
            }
            break;
        }

        outToken.push_back(static_cast<char>(c));
        ++k;
    }

    if (!sawHyphenInside) return false;

    end = k;
    return !outToken.empty();
}

void Tokenizador::Tokenizar(const std::string& str, std::list<std::string>& tokens) const {
    tokens.clear();

    const std::string s = NormalizaCadena(str);

    if (!casosEspeciales) {
        // Recorrido lineal usando delimTable; los saltos de línea siempre separan.
        std::string current;
        current.reserve(32);

        for (std::size_t i = 0; i < s.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(s[i]);

            bool isSep = false;
            if (c == '\n' || c == '\r') isSep = true;
            else if (EsDelimitador(c)) isSep = true;

            if (isSep) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current.push_back(static_cast<char>(c));
            }
        }
        if (!current.empty()) tokens.push_back(current);
        return;
    }

    // casosEspeciales == true
    std::size_t i = 0;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);

        if (EsSeparadorCasosEspeciales(c)) {
            bool canStartLeadingDecimal = false;

            if ((c == '.' || c == ',') &&
                (i + 1 < s.size()) &&
                is_digit(static_cast<unsigned char>(s[i + 1])) &&
                EsDelimitador(c)) {

                if (c == '.' && i >= 2 &&
                    static_cast<unsigned char>(s[i - 1]) == '.' &&
                    static_cast<unsigned char>(s[i - 2]) == '.') {
                    canStartLeadingDecimal = false;
                } else if (i == 0) {
                    canStartLeadingDecimal = true;
                } else {
                    unsigned char prev = static_cast<unsigned char>(s[i - 1]);
                    if (EsSeparadorCasosEspeciales(prev)) canStartLeadingDecimal = true;
                }
            }

            if (!canStartLeadingDecimal) {
                ++i;
                continue;
            }
    
        }

        std::size_t end = i;

        // 1) URL
        if (ParseURL(s, i, end)) {
            if (end > i) tokens.push_back(s.substr(i, end - i));
            i = end;
            continue;
        }

        // 2) Números con '.' / ','
        {
            std::string tok;
            std::string pending;
            if (ParseNumero(s, i, end, tok, pending)) {
                if (!tok.empty()) tokens.push_back(tok);
                i = end;

                // Si terminamos por %/$ seguido de blanco, emitimos ese símbolo como token.
                if (!pending.empty()) {
                    // Consumir el símbolo.
                    if (i < s.size() && s[i] == pending[0]) ++i;
                    tokens.push_back(pending);
                }
                continue;
            }
        }

        // 3) E-mail
        if (ParseEmail(s, i, end)) {
            if (end > i) tokens.push_back(s.substr(i, end - i));
            i = end;
            continue;
        }

        // 4) Acronyms with '.'
        {
            std::string tok;
            if (ParseAcronimo(s, i, end, tok)) {
                if (!tok.empty()) tokens.push_back(tok);
                i = end;
                continue;
            }
        }

        // 5) Hyphenated multi-words
        {
            std::string tok;
            if (ParseMultipalabraGuiones(s, i, end, tok)) {
                if (!tok.empty()) tokens.push_back(tok);
                i = end;
                continue;
            }
        }

        // Caso por defecto: consumir hasta el siguiente separador.
        std::string current;
        while (i < s.size()) {
            unsigned char ch = static_cast<unsigned char>(s[i]);
            if (EsSeparadorCasosEspeciales(ch)) break;
            current.push_back(static_cast<char>(ch));
            ++i;
        }
        if (!current.empty()) tokens.push_back(current);
    }
}

bool Tokenizador::Tokenizar(const std::string& NomFichEntr, const std::string& NomFichSal) const {
    // El código base del enunciado para tokenizar ficheros se mantiene a nivel conceptual,
    // pero se mejora para no guardar TODOS los tokens en memoria de golpe.

    std::ifstream i;
    std::ofstream f;
    std::string cadena;
    std::list<std::string> tokens;

    i.open(NomFichEntr.c_str());
    if (!i) {
        std::cerr << "ERROR: No existe el archivo: " << NomFichEntr << '\n';
        return false;
    } else {
        f.open(NomFichSal.c_str());
        if (!f) {
            std::cerr << "ERROR: No se puede abrir el archivo de salida: " << NomFichSal << '\n';
            i.close();
            return false;
        }

        // Buffer grande de salida para reducir syscalls (se nota mucho en WSL/sistema de ficheros).
        static char outbuf[1 << 20]; // 1 MiB
        f.rdbuf()->pubsetbuf(outbuf, sizeof(outbuf));

        while (std::getline(i, cadena)) {
            if (cadena.length() != 0) {
                Tokenizar(cadena, tokens);
                std::string out;
                out.reserve(256 * tokens.size());
                for (std::list<std::string>::const_iterator itS = tokens.begin(); itS != tokens.end(); ++itS) {
                    out.append(*itS);
                    out.push_back('\n');
                }
                f.write(out.data(), static_cast<std::streamsize>(out.size()));
            }
        }
    }

    i.close();
    f.close();
    return true;
}

bool Tokenizador::Tokenizar(const std::string& i) const {
    return Tokenizar(i, i + ".tk");
}

bool Tokenizador::TokenizarListaFicheros(const std::string& i) const {
    std::ifstream in(i.c_str());
    if (!in) {
        std::cerr << "ERROR: No existe el archivo: " << i << '\n';
        return false;
    }

    bool ok = true;
    std::string ruta;
    while (std::getline(in, ruta)) {
        StripCR(ruta);
        if (ruta.empty()) continue;
        struct stat st;
        if (stat(ruta.c_str(), &st) == -1) {
            std::cerr << "ERROR: No existe el archivo: " << ruta << '\n';
            ok = false;
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            std::cerr << "ERROR: Es un directorio: " << ruta << '\n';
            ok = false;
            continue;
        }

        if (!Tokenizar(ruta)) {
            ok = false;
        }
    }

    return ok;
}

bool Tokenizador::TokenizarDirectorio(const std::string& dir) const {
    struct stat st;
    if (stat(dir.c_str(), &st) == -1 || !S_ISDIR(st.st_mode)) {
        std::cerr << "ERROR: No existe el directorio: " << dir << '\n';
        return false;
    }

    bool ok = true;

    // Recorrido en profundidad (DFS) usando opendir/readdir.
    DIR* dp = opendir(dir.c_str());
    if (!dp) {
        std::cerr << "ERROR: No se puede abrir el directorio: " << dir << '\n';
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dp)) != 0) {
        const char* name = entry->d_name;
        if (!name) continue;
        if (std::string(name) == "." || std::string(name) == "..") continue;

        std::string path = dir;
        if (!path.empty() && path[path.size() - 1] != '/') path.push_back('/');
        path += name;

        struct stat st2;
        if (stat(path.c_str(), &st2) == -1) {
            std::cerr << "ERROR: No existe el archivo: " << path << '\n';
            ok = false;
            continue;
        }

        if (S_ISDIR(st2.st_mode)) {
            if (!TokenizarDirectorio(path)) ok = false;
        } else {
            if (!Tokenizar(path)) ok = false;
        }
    }

    closedir(dp);
    return ok;
}