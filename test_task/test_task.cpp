#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct ListNode {
    ListNode* prev = nullptr;
    ListNode* next = nullptr;
    ListNode* rand = nullptr;
    std::string data;

    ListNode() = default;
    ListNode(const std::string& d) : data(d) {}
};

class ListSerializer {
public:
    // Чтение списка из текстового файла
    ListNode* readFromTextFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        // Читаем все строки
        std::vector<std::string> allData;
        std::vector<int> randIndices;
        std::string line;

        while (std::getline(file, line)) {
            size_t separatorPos = line.find(';');
            if (separatorPos == std::string::npos) {
                continue;
            }

            allData.push_back(line.substr(0, separatorPos));
            randIndices.push_back(std::stoi(line.substr(separatorPos + 1)));
        }
        file.close();

        // Создаем узлы
        std::vector<ListNode*> allNodes;
        for (const std::string& data : allData) {
            allNodes.push_back(new ListNode(data));
        }

        // Устанавливаем связи
        for (size_t i = 0; i < allNodes.size(); i++) {
            if (i > 0) {
                allNodes[i]->prev = allNodes[i - 1];
            }
            if (i + 1 < allNodes.size()) {
                allNodes[i]->next = allNodes[i + 1];
            }

            int randIndex = randIndices[i];
            if (randIndex >= 0 && randIndex < (int)allNodes.size()) {
                allNodes[i]->rand = allNodes[randIndex];
            }
        }

        // Сохраняем для последующего использования
        m_allNodes = allNodes;
        return allNodes.empty() ? nullptr : allNodes[0];
    }

    // Сохранение списка в бинарный файл
    void saveToBinaryFile(const std::string& filename, ListNode* head) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot create file: " + filename);
        }

        // Пустой список
        if (head == nullptr) {
            uint32_t zero = 0;
            file.write((char*)&zero, sizeof(zero));
            file.close();
            return;
        }

        // Собираем все узлы в вектор
        m_allNodes.clear();
        ListNode* current = head;
        while (current != nullptr) {
            m_allNodes.push_back(current);
            current = current->next;
        }

        // Пишем количество узлов
        uint32_t nodeCount = m_allNodes.size();
        file.write((char*)&nodeCount, sizeof(nodeCount));

        // Вычисляем позиции узлов в файле
        std::vector<size_t> nodePositions(m_allNodes.size());
        size_t currentPos = sizeof(uint32_t);

        for (size_t i = 0; i < m_allNodes.size(); i++) {
            nodePositions[i] = currentPos;
            currentPos += sizeof(size_t) * 3 + sizeof(uint32_t) + m_allNodes[i]->data.size();
        }

        // Пишем каждый узел
        for (size_t i = 0; i < m_allNodes.size(); i++) {
            ListNode* node = m_allNodes[i];

            // Получаем позиции связанных узлов
            size_t prevPos = 0, nextPos = 0, randPos = 0;

            // Ищем позицию prev узла
            for (size_t j = 0; j < m_allNodes.size(); j++) {
                if (m_allNodes[j] == node->prev) {
                    prevPos = nodePositions[j];
                    break;
                }
            }

            // Ищем позицию next узла
            for (size_t j = 0; j < m_allNodes.size(); j++) {
                if (m_allNodes[j] == node->next) {
                    nextPos = nodePositions[j];
                    break;
                }
            }

            // Ищем позицию rand узла
            for (size_t j = 0; j < m_allNodes.size(); j++) {
                if (m_allNodes[j] == node->rand) {
                    randPos = nodePositions[j];
                    break;
                }
            }

            // Пишем три позиции
            file.write((char*)&prevPos, sizeof(prevPos));
            file.write((char*)&nextPos, sizeof(nextPos));
            file.write((char*)&randPos, sizeof(randPos));

            // Пишем данные
            uint32_t dataSize = node->data.size();
            file.write((char*)&dataSize, sizeof(dataSize));
            file.write(node->data.c_str(), dataSize);
        }

        file.close();
    }

    // Освобождение памяти
    void deleteList(ListNode* head) {
        while (head != nullptr) {
            ListNode* next = head->next;
            delete head;
            head = next;
        }
    }

private:
    std::vector<ListNode*> m_allNodes;
};

int main() {
    try {
        ListSerializer serializer;

        std::cout << "Reading from inlet.in..." << std::endl;
        ListNode* head = serializer.readFromTextFile("inlet.in");

        std::cout << "Saving to outlet.out..." << std::endl;
        serializer.saveToBinaryFile("outlet.out", head);

        serializer.deleteList(head);

        std::cout << "Serialization completed." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}