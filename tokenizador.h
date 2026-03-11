// RAÚL RUIZ FLORES, 50593447W

#ifndef TOKENIZADOR_H
#define TOKENIZADOR_H

#include <string>
#include <list>
#include <ostream>


class Tokenizador {
friend std::ostream& operator<<(std::ostream&, const Tokenizador&);	 
// cout << "DELIMITADORES: " << delimiters << " TRATA CASOS ESPECIALES: " << casosEspeciales << " PASAR A MINUSCULAS Y SIN ACENTOS: " << pasarAminuscSinAcentos;
// Aunque se modifique el almacenamiento de los delimitadores por temas de eficiencia, el campo delimiters se imprimir� con el string le�do en el tokenizador (tras las modificaciones y eliminaci�n de los caracteres repetidos correspondientes)

public:
    Tokenizador (const std::string& delimitadoresPalabra, const bool& kcasosEspeciales, const bool& minuscSinAcentos);	
    // Inicializa delimiters a delimitadoresPalabra filtrando que no se introduzcan delimitadores repetidos (de izquierda a derecha, en cuyo caso se eliminar�an los que hayan sido repetidos por la derecha); casosEspeciales a kcasosEspeciales; pasarAminuscSinAcentos a minuscSinAcentos

    Tokenizador (const Tokenizador&);

    Tokenizador ();	
    // Inicializa delimiters=",;:.-/+*\\ '\"{}[]()<>�!�?&#=\t@"; casosEspeciales a true; pasarAminuscSinAcentos a false

    ~Tokenizador ();	// Pone delimiters=""

    Tokenizador& operator= (const Tokenizador&);

    void Tokenizar (const std::string& str, std::list<std::string>& tokens) const;
    // Tokeniza str devolviendo el resultado en tokens. La lista tokens se vaciar� antes de almacenar el resultado de la tokenizaci�n. 

    bool Tokenizar (const std::string& i, const std::string& f) const; 
    // Tokeniza el fichero i guardando la salida en el fichero f (una palabra en cada l�nea del fichero). Devolver� true si se realiza la tokenizaci�n de forma correcta; false en caso contrario enviando a cerr el mensaje correspondiente (p.ej. que no exista el archivo i)

    bool Tokenizar (const std::string & i) const;
    // Tokeniza el fichero i guardando la salida en un fichero de nombre i a�adi�ndole extensi�n .tk (sin eliminar previamente la extensi�n de i por ejemplo, del archivo pp.txt se generar�a el resultado en pp.txt.tk), y que contendr� una palabra en cada l�nea del fichero. Devolver� true si se realiza la tokenizaci�n de forma correcta; false en caso contrario enviando a cerr el mensaje correspondiente (p.ej. que no exista el archivo i)

    bool TokenizarListaFicheros (const std::string& i) const; 
    // Tokeniza el fichero i que contiene un nombre de fichero por l�nea guardando la salida en ficheros (uno por cada l�nea de i) cuyo nombre ser� el le�do en i a�adi�ndole extensi�n .tk, y que contendr� una palabra en cada l�nea del fichero le�do en i. Devolver� true si se realiza la tokenizaci�n de forma correcta de todos los archivos que contiene i; devolver� false en caso contrario enviando a cerr el mensaje correspondiente (p.ej. que no exista el archivo i, o que se trate de un directorio, enviando a "cerr" los archivos de i que no existan o que sean directorios; luego no se ha de interrumpir la ejecuci�n si hay alg�n archivo en i que no exista)

    bool TokenizarDirectorio (const std::string& i) const; 
    // Tokeniza todos los archivos que contenga el directorio i, incluyendo los de los subdirectorios, guardando la salida en ficheros cuyo nombre ser� el de entrada a�adi�ndole extensi�n .tk, y que contendr� una palabra en cada l�nea del fichero. Devolver� true si se realiza la tokenizaci�n de forma correcta de todos los archivos; devolver� false en caso contrario enviando a cerr el mensaje correspondiente (p.ej. que no exista el directorio i, o los ficheros que no se hayan podido tokenizar)

    void DelimitadoresPalabra(const std::string& nuevoDelimiters); 
    // Inicializa delimiters a nuevoDelimiters, filtrando que no se introduzcan delimitadores repetidos (de izquierda a derecha, en cuyo caso se eliminar�an los que hayan sido repetidos por la derecha)

    void AnyadirDelimitadoresPalabra(const std::string& nuevoDelimiters); // 
    // A�ade al final de "delimiters" los nuevos delimitadores que aparezcan en "nuevoDelimiters" (no se almacenar�n caracteres repetidos)

    std::string DelimitadoresPalabra() const; 
    // Devuelve "delimiters" 

    void CasosEspeciales (const bool& nuevoCasosEspeciales);
    // Cambia la variable privada "casosEspeciales" 

    bool CasosEspeciales ();
    // Devuelve el contenido de la variable privada "casosEspeciales" 

    void PasarAminuscSinAcentos (const bool& nuevoPasarAminuscSinAcentos);
    // Cambia la variable privada "pasarAminuscSinAcentos". Atenci�n al formato de codificaci�n del corpus (comando "file" de Linux). Para la correcci�n de la pr�ctica se utilizar� el formato actual (ISO-8859). 

    bool PasarAminuscSinAcentos ();
    // Devuelve el contenido de la variable privada "pasarAminuscSinAcentos"



private:
    std::string delimiters;		// Delimitadores de t�rminos. Aunque se modifique la forma de almacenamiento interna para mejorar la eficiencia, este campo debe permanecer para indicar el orden en que se introdujeron los delimitadores

    bool casosEspeciales;
    // Si true detectar� palabras compuestas y casos especiales. Sino, trabajar� al igual que el algoritmo propuesto en la secci�n "Versi�n del tokenizador vista en clase"

    bool pasarAminuscSinAcentos;
    // Si true pasar� el token a min�sculas y quitar� acentos, antes de realizar la tokenizaci�n

    bool delimTable[256]; 
    // Tabla de delimitadores lookup para O(1)
private:
    void RebuildDelimiterTable();

    static std::string FiltrarRepetidos(const std::string& s);

    static bool EsBlanco(unsigned char c);

    bool EsDelimitador(unsigned char c) const;

    bool EsSeparadorCasosEspeciales(unsigned char c) const;

    static unsigned char NormalizaCharISO8859_1(unsigned char c);

    std::string NormalizaCadena(const std::string& s) const;

    bool ParseURL(const std::string& s, std::size_t pos, std::size_t& end) const;
    // Parsing helpers. Devuelve true si parsed token especial en pos.
   
    bool ParseNumero(const std::string& s, std::size_t pos, std::size_t& end, std::string& outToken, std::string& pendingSymbol) const;

    bool ParseEmail(const std::string& s, std::size_t pos, std::size_t& end) const;

    bool ParseAcronimo(const std::string& s, std::size_t pos, std::size_t& end, std::string& outToken) const;

    bool ParseMultipalabraGuiones(const std::string& s, std::size_t pos, std::size_t& end, std::string& outToken) const;
};

#endif // TOKENIZADOR_H
