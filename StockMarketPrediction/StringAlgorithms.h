#ifndef STRING_ALGORITHMS_HEADER
#define STRING_ALGORITHMS_HEADER

#include <Windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdarg.h>
#include <vector>
#include <bitset>

using namespace std;

string ToUpperCase(string input)
{
	int length = (int)input.length();

	stringstream output;

	int i = -1;
	while (++i < (int)input.length())
	{
		if (input[i] >= 'a' && input[i] <= 'z')
			output << (char)(input[i] - ' ');
		else
			output << (char)input[i];
	}

	return output.str();
}

char * ToUpperCase(char * input)
{
	int i = 0, length = 0;
	do length++; while (input[i++] != '\0');

	char * output = new char[length];

	i = -1;
	while (input[++i] != '\0')
	{
		output[i] = input[i];
		if (input[i] >= 'a' && input[i] <= 'z')
			output[i] -= ' ';
	}
	output[length - 1] = '\0';

	return output;
}

void ToLowerCase(string * input)
{
	stringstream output;

	int i = -1;
	while (++i < (int)input->length())
	{
		if ((*input)[i] >= 'A' && (*input)[i] <= 'Z')
			output << (char)((*input)[i] + ' ');
		else
			output << (char)(*input)[i];
	}

	input->assign(output.str());
}

int HexStringToInt(string input)
{
	stringstream ss; unsigned int output;

	ss << hex << input.substr(2, input.length() - 2);
	ss >> output;

	return output;
}

bool CompareStrings(string a, string b)
{
	for (int i = 0; i < (int)a.length(); i++)
	{
		if (i >= (int)b.length())
			return true;
		if (a[i] != b[i])
			return false;
	}
	return true;
}

int LastIndexOf(string container, string contains)
{
	if (container.length() < contains.length())
		return -1;

	for (int j = (int)container.length() - (int)contains.length(); j >= 0; j--)
	{
		for (int k = 0; k < (int)contains.length(); k++)
		{
			if (container[j + k] != contains[k])
				break;
			else if (k + 1 != contains.length())
				return j + k + 1;
		}
	}

	return -1;
}

string AddCommas(string input)
{
	for (int i = (int)input.length() - 1, j = 1; i > 0; i--, j++)
		if (j % 3 == 0)
			input = input.insert(i, ",");

	return input;
}

string RemoveNonNumericalCharacters(string input)
{
	int index;
	while ((index = (int)input.find_first_not_of("01234567890.")) != -1)
		input.erase(index, 1);

	return input;
}

bool HexStringToByteArray(string input, BYTE * output, int * length)
{
	string hexValues = "0123456789abcdef";

	input.erase(std::remove_if(input.begin(), input.end(), isspace), input.end());

	if (input.length() % 2 != 0)
		return false;

	*length = (int)input.length() / 2;

	int counter = 0;
	for (int i = 1; i < (int)input.length(); i += 4)
	{
		int base = 1;
		int out = 0;
		for (int j = 0; j < 2; j++)
		{
			int number = (int)hexValues.find(input[i]);
			out += number * base;
			base *= 16;
			i--;
		}
		output[counter++] = out;
	}

	return true;
}

bool StringContains(char * container, char * contains)
{
	int i = 0;
	while (container[i++] != '\0');
	int containerLength = i - 1;
	i = 0;
	while (contains[i++] != '\0');
	int containsLength = i - 1;

	for (int j = 0; j < containerLength; j++)
		for (int k = 0; k < containsLength; k++)
			if (container[j] == contains[k] && k == containsLength - 1)
				return true;

	return false;
}

int IndexOf(string container, string contains, int startAt, char wildcard)
{
	if (container.length() < contains.length())
		return -1;

	for (int j = startAt; j < (int)container.length() - (int)contains.length(); j++)
	{
		for (int k = 0; k < (int)contains.length(); k++)
		{
			if (contains[k] == wildcard && k + 1 == (int)contains.length())
				return j + k + 1;
			else if (contains[k] == wildcard)
				continue;
			else if (container[j + k] != contains[k])
				break;
			else if (k + 1 == (int)contains.length())
				return j + k + 1;
		}
	}

	return -1;
}

int IndexOf(string container, string contains, int startAt)
{
	if (container.length() < contains.length())
		return -1;

	for (int j = startAt; j < (int)container.length() - (int)contains.length(); j++)
	{
		for (int k = 0; k < (int)contains.length(); k++)
		{
			if (container[j + k] != contains[k])
				break;
			else if (k + 1 == (int)contains.length())
				return j + k + 1;
		}
	}

	return -1;
}

void RemoveCharacter(string * input, char character)
{
	for (int i = 0; i < (int)input->length(); i++)
		if ((*input)[i] == character)
			input->erase(i--, 1);
}

string RemoveCharacter(string input, char character)
{
	for (int i = 0; i < (int)input.length(); i++)
		if (input[i] == character)
			input.erase(i--, 1);
	return input;
}

void ReplaceCharacter(string * input, char characterToReplace, char characterToReplaceWith)
{
	for (int i = 0; i < (int)input->length(); i++)
		if ((*input)[i] == characterToReplace)
			(*input)[i] = characterToReplaceWith;
}

string ReplaceCharacter(string input, char characterToReplace, char characterToReplaceWith)
{
	for (int i = 0; i < (int)input.length(); i++)
		if (input[i] == characterToReplace)
			input[i] = characterToReplaceWith;

	return input;
}

void RemoveTrailingSpaces(string * input)
{
	int index = (int)input->find_last_not_of(" ");
	if (index != (int)input->length() - 1)
		input->erase(index, (int)input->length() - index);
}

void RemovePreceedingSpaces(string * input)
{
	int index = (int)input->find_first_not_of(" ");
	if (index != 0)
		input->erase(0, index);
}

string RemoveExtraneousCharacter(string input)
{
	for (int i = 0; i < (int)input.length(); i++)
		if (input[i] < 32 || input[i] >= 127)
			input = input.erase(i, 1);
	return input;
}

string TruncateString(string input, int length)
{
	if ((int)input.length() >= length)
	{
		input = input.substr(0, length - 3);
		input = input.append("...");
	}

	return input;
}

string HexDump(UCHAR * input, int length, int addressBase = 0, int displayWidth = 16, bool binary = false)
{
	stringstream ss;

	int numberOfLines = length / displayWidth;
	if (length % displayWidth != 0)
		numberOfLines++;

	for (int i = 0; i < (numberOfLines * displayWidth); i++)
	{
		if (i % displayWidth == 0)
		{
			ss << "0x";
			ss.width(8);
			ss.fill('0');
			ss << hex << addressBase + i << "  ";
		}
		if ((i + 1) % displayWidth == 0)
		{
			if (i < length)
				if (binary)
					ss << bitset<8>((int)input[i]) << "  ";
				else
				{
					ss.width(2);
					ss.fill('0');
					ss << hex << (int)input[i] << "  ";
				}
			else
				ss << "    ";
			for (int j = (i + 1) - displayWidth; j < (i + 1); j++)
				if (j < length)
					if (input[j] > 32 && input[j] < 127)
						ss << input[j];
					else
						ss << ".";
				else
					ss << " ";
			ss << endl;
		}
		else if ((i + 1) % 8 == 0)
		{
			if (i < length)
			{
				ss.width(2);
				ss.fill('0');
				if (binary)
					ss << bitset<8>((int)input[i]) << "  ";
				else
					ss << hex << (int)input[i] << "  ";
			}
			else
				ss << "    ";
		}
		else
		{
			if (i < length)
			{
				ss.width(2);
				ss.fill('0');
				if (binary)
					ss << bitset<8>((int)input[i]) << " ";
				else
					ss << hex << (int)input[i] << " ";
			}
			else
				ss << "   ";
		}
	}
	return ss.str();
}

string InsertToString(string input, ...)
{
	int index = -1, result, count = 0;
	while ((index = (int)input.find("%s", ++index)) != -1)
		count++;

	va_list List;
	va_start(List, input);

	for (int i = 0; i < count; i++)
	{
		input = input.erase((result = (int)input.find("%s")), 2);
		input = input.insert(result, va_arg(List, char *));
	}

	va_end(List);

	return input;
}

string RemoveTrailingSpaces(string input)
{
	int index = (int)input.find_last_not_of(" ");
	if (index != (int)input.length() - 1)
		input.erase(index + 1, (int)input.length() - index);

	return input;
}

void NormalizeFilename(string * filename)
{
	for (int i = 0; i < (int)filename->size(); i++)
		if ((*filename)[i] == '/' || (*filename)[i] == '\\' || (*filename)[i] == '?' || (*filename)[i] == ':' || (*filename)[i] == '*' || (*filename)[i] == '|' || (*filename)[i] == '\"' || (*filename)[i] == '<' || (*filename)[i] == '>')
			(*filename)[i] = '_';
}

string NormalizeFilename(string filename)
{
	for (int i = 0; i < (int)filename.size(); i++)
		if (filename[i] == '/' || filename[i] == '\\' || filename[i] == '?' || filename[i] == ':' || filename[i] == '*' || filename[i] == '|' || filename[i] == '\"' || filename[i] == '<' || filename[i] == '>')
			filename[i] = '_';

	return filename;
}

vector<string> SplitString(string input, string splitBy)
{
	vector<string> output;

	int index1 = 0, index2 = 0;

	while ((index2 = (int)input.find(splitBy, index2 + 1)) != -1)
	{
		output.push_back(input.substr(index1, index2 - index1));
		index1 = index2 + (int)splitBy.length();
	}

	output.push_back(input.substr(index1, input.length() - index1));

	return output;
}

bool StringContainsNumber(string container)
{
	if (container.find_first_of("1234567890") != -1)
		return true;
	return false;
}

bool StringContainsSymbol(string container)
{
	if (container.find_first_of("`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?") != -1)
		return true;
	return false;
}

#endif