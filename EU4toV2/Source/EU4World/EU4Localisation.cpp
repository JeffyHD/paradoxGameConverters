#include "EU4Localisation.h"

#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>

void EU4Localisation::ReadFromFile(const std::string& fileName)
{
	std::ifstream in(fileName);

	const int maxLineLength = 10000;
	char line[maxLineLength];

	// First line is the language like "l_english:"
	in.getline(line, maxLineLength);
	std::string language = DetermineLanguageForFile(RemoveUTF8BOM(line));
	if (language.empty())
	{
		return;
	}

	// Subsequent lines are 'KEY: "Text"'
	while (!in.eof())
	{
		in.getline(line, maxLineLength);
		if (!in.eof())
		{
			auto keyLocalisationPair = DetermineKeyLocalisationPair(RemoveUTF8BOM(line));
			const std::string& key = keyLocalisationPair.first;
			const std::string& currentLocalisation = keyLocalisationPair.second;
			if (!key.empty() && !currentLocalisation.empty())
			{
				localisations[key][language] = currentLocalisation;
			}
		}
	}
}

void EU4Localisation::ReadFromAllFilesInFolder(const std::string& folderPath)
{
	if (boost::filesystem::exists(folderPath) && boost::filesystem::is_directory(folderPath))
	{
		for (boost::filesystem::directory_iterator dir_iter(folderPath);
			dir_iter != boost::filesystem::directory_iterator();
			++dir_iter)
		{
			if (boost::filesystem::is_regular_file(dir_iter->status()))
			{
				ReadFromFile(dir_iter->path().string());
			}
		}
	}
}

const std::string& EU4Localisation::GetText(const std::string& key, const std::string& language) const
{
	static const std::string noLocalisation = "";

	auto keyFindIter = localisations.find(key);
	if (keyFindIter == localisations.end())
	{
		return noLocalisation;
	}
	const auto& localisationsByLanguage = keyFindIter->second;
	auto languageFindIter = localisationsByLanguage.find(language);
	if (languageFindIter == localisationsByLanguage.end())
	{
		return noLocalisation;
	}

	return languageFindIter->second;
}

const std::map<std::string, std::string>& EU4Localisation::GetTextInEachLanguage(const std::string& key) const
{
	static const std::map<std::string, std::string> noLocalisation;

	auto keyFindIter = localisations.find(key);
	if (keyFindIter == localisations.end())
	{
		return noLocalisation;
	}

	return keyFindIter->second;
}

std::string EU4Localisation::DetermineLanguageForFile(const std::string& text)
{
	static const std::string noLanguageIndicated = "";

	if (text.size() < 2 || text[0] != 'l' || text[1] != '_')
	{	// Not in the desired format - no "l_"
		return noLanguageIndicated;
	}
	size_t beginPos = 2;	// Skip l_ for our language name.
	size_t endPos = text.find(':', beginPos);
	if (endPos == std::string::npos)
	{	// Not in the desired format - no ":"
		return noLanguageIndicated;
	}

	return text.substr(beginPos, endPos - beginPos);
}

std::pair<std::string, std::string> EU4Localisation::DetermineKeyLocalisationPair(const std::string& text)
{
	static const std::pair<std::string, std::string> noLocalisationPair;

	size_t keyBeginPos = text.find_first_not_of(' ');
	if (keyBeginPos == std::string::npos)
	{
		return noLocalisationPair;
	}
	size_t keyEndPos = text.find_first_of(':', keyBeginPos + 1);
	size_t quotePos = text.find_first_of('"', keyEndPos);
	if (quotePos == std::string::npos)
	{
		return noLocalisationPair;
	}
	size_t localisationBeginPos = quotePos + 1;
	size_t localisationEndPos = text.find_first_of('"', localisationBeginPos);
	return std::make_pair(text.substr(keyBeginPos, keyEndPos - keyBeginPos), text.substr(localisationBeginPos, localisationEndPos - localisationBeginPos));
}

std::string EU4Localisation::RemoveUTF8BOM(const std::string& text)
{
	if (text.size() >= 3 && text[0] == '\xEF' && text[1] == '\xBB' && text[2] == '\xBF')
	{
		return text.substr(3);
	}
	else
	{
		return text;
	}
}
