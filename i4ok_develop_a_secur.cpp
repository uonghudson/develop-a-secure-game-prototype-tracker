#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cryptopp/aes.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>

using namespace std;
using namespace CryptoPP;

class GamePrototypeTracker {
private:
    vector<string> gamePrototypes;
    string password;
    string encryptionKey;

    string hashPassword(const string& password) {
        SHA256 sha256;
        byte digest[CryptoPP::SHA256::DIGESTSIZE];
        sha256.CalculateDigest(digest, (byte*)password.c_str(), password.size());
        string hashedPassword;
        HexEncoder encoder;
        encoder.Attach(new StringSink(hashedPassword));
        encoder.Put(digest, sizeof(digest));
        encoder.MessageEnd();
        return hashedPassword;
    }

    string encryptData(const string& data) {
        AES aes;
        aes.SetKey((byte*)encryptionKey.c_str(), encryptionKey.size());
        string encryptedData;
        StringSource(data, true,
            new StreamTransformationFilter(aes, new HexEncoder(new StringSink(encryptedData))));
        return encryptedData;
    }

    string decryptData(const string& data) {
        AES aes;
        aes.SetKey((byte*)encryptionKey.c_str(), encryptionKey.size());
        string decryptedData;
        StringSource(data, true,
            new HexDecoder(new StreamTransformationFilter(aes, new StringSink(decryptedData))));
        return decryptedData;
    }

public:
    GamePrototypeTracker(const string& password) {
        this->password = hashPassword(password);
        this->encryptionKey = password.substr(0, 16);
    }

    void addGamePrototype(const string& gamePrototype) {
        gamePrototypes.push_back(gamePrototype);
        saveGamePrototypes();
    }

    void removeGamePrototype(const string& gamePrototype) {
        auto it = find(gamePrototypes.begin(), gamePrototypes.end(), gamePrototype);
        if (it != gamePrototypes.end()) {
            gamePrototypes.erase(it);
            saveGamePrototypes();
        }
    }

    void saveGamePrototypes() {
        string encryptedData;
        for (const auto& gamePrototype : gamePrototypes) {
            encryptedData += encryptData(gamePrototype) + "\n";
        }
        ofstream file("game_prototypes.txt");
        file << encryptedData;
        file.close();
    }

    void loadGamePrototypes() {
        ifstream file("game_prototypes.txt");
        string data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        vector<string> decryptedPrototypes;
        string decryptedData;
        for (const auto& line : split(data, "\n")) {
            decryptedData = decryptData(line);
            decryptedPrototypes.push_back(decryptedData);
        }
        gamePrototypes = decryptedPrototypes;
    }

    vector<string> getGamePrototypes() {
        return gamePrototypes;
    }

private:
    vector<string> split(const string& str, const string& delim) {
        vector<string> tokens;
        size_t prev = 0, pos = 0;
        do {
            pos = str.find(delim, prev);
            if (pos == string::npos) pos = str.length();
            string token = str.substr(prev, pos - prev);
            if (!token.empty()) tokens.push_back(token);
            prev = pos + delim.length();
        } while (pos < str.length());
        return tokens;
    }
};

int main() {
    GamePrototypeTracker tracker("mysecretpassword");
    tracker.addGamePrototype("Game Prototype 1");
    tracker.addGamePrototype("Game Prototype 2");
    tracker.addGamePrototype("Game Prototype 3");
    tracker.loadGamePrototypes();
    for (const auto& gamePrototype : tracker.getGamePrototypes()) {
        cout << gamePrototype << endl;
    }
    return 0;
}