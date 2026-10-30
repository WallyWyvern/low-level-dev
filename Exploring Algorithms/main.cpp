#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>


int main() {
    // gebruik functies uit <algorithm> en <functional> om de volgende opdrachten uit te voeren:

    // splits de vector in 2 nieuwe vectoren: 1 met alles wat alfabetisch voor 'purple' komt, 1 met alles er na
    // https://stackoverflow.com/questions/9811235/best-way-to-split-a-vector-into-two-smaller-arrays
    {
        std::vector<std::string> colours{"red", "green", "white", "blue", "orange", "green", "orange", "black", "purple"};
        std::sort(colours.begin(), colours.end());
        std::vector<std::string> col1(colours.begin(), colours.end() - 3 );
        std::vector<std::string> col2(colours.end() - 3, colours.end() );
        std::cout << "Before purple: ";
        for (int i = 0; i < col1.size(); i++)
        {
            std::cout << col1[i] << " ";
        }
        std::cout << std::endl;
        std::cout << "After purple: ";
        for (int i = 0; i < col2.size(); i++)
        {
            std::cout << col2[i] << " ";
        }
        std::cout << std::endl;
    }

    // maak alle elementen UPPERCASE
    // https://stackoverflow.com/questions/735204/convert-a-string-in-c-to-upper-case
    {
        std::vector<std::string> colours{"red", "green", "white", "blue", "orange", "green", "orange", "black", "purple"};
        std::cout << "ALL UPPERCASE: ";
        for (int i = 0; i < colours.size(); i++)
        {
            std::transform(colours[i].begin(), colours[i].end(), colours[i].begin(), ::toupper);
            std::cout << colours[i] << " ";
        }
        std::cout << std::endl;
    }

    // verwijder alle dubbele elementen
    // https://stackoverflow.com/questions/1041620/whats-the-most-efficient-way-to-erase-duplicates-and-sort-a-vector
    {
        std::vector<std::string> colours{"red", "green", "white", "blue", "orange", "green", "orange", "black", "purple"};
        std::sort(colours.begin(), colours.end());
        colours.erase(unique(colours.begin(), colours.end()), colours.end());
        std::cout << "Unique colours: ";
        for (int i = 0; i < colours.size(); i++)
        {
            std::cout << colours[i] << " ";
        }
        std::cout << std::endl;
    }

    // verwijder alle negatieve elementen
    {
        std::vector<double> numbers{10, 324422, 6, -23, 234.5, 654.1, 3.1242, -9.23, 635};
        for (int i = 0; i < numbers.size(); i++)
        {
            if (numbers[i] < 0)
            {
                numbers.erase(numbers.begin() + i);
            }
        }
        std::cout << "no negative numbers: ";
        for (int i = 0; i < numbers.size(); i++)
        {
            std::cout << numbers[i] << " ";
        }
        std::cout << std::endl;
    }

    // bepaal voor alle elementen of ze even of oneven zijn
    // used this for implementation: https://stackoverflow.com/questions/9612839/is-there-a-way-to-check-if-a-variable-is-a-whole-number-c
    {
        std::vector numbers{10, 324422, 6, -23, 234, 654, 3, -9, 635};
        std::cout << "Even/uneven check: ";
        for (int i = 0; i < numbers.size(); i++)
        {
            double temp = static_cast<double>(numbers[i])/2;
            if (std::fmod(temp, 1) == 0) // if a number is not whole, does not return 0, aka is odd number
            {
                std::cout << numbers[i] << " is even, ";
            } else
            {
                std::cout << numbers[i] << " is odd, ";
            }
        }
        std::cout << std::endl;
    }

    // bepaal de som, het gemiddelde, en het product van alle getallen te berekenen
    {
        std::vector<double> numbers{10, 324422.1, 6, -23, 234.5, 654.1, 3.1242, -9.23, 635};
        double sum = std::reduce(numbers.begin(), numbers.end(), 1.0);
        double product = std::reduce(numbers.begin(), numbers.end(), 1.0, std::multiplies<double>());
        double average = sum / numbers.size();
        std::cout << "Sum: " << sum << ", Product: " << product << ", Average: " << average << std::endl;
    }

    return 0;
}
