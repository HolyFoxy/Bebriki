// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "CodeGenerator.h"

CodeGenerator::CodeGenerator()
{
	m_out.open("out.txt");

	m_j = 0;
	m_temp_point = 10;
	m_i = 0;
	m_var_num = 0;

	m_flag = false;
}

void CodeGenerator::CodeCreator(std::vector<std::vector<std::string>>& lines) 
{
	m_out << ".model small\n\n.data?\n\n.data\n\n.code\n\nStart:\n\n";
	m_lines = lines;
	while (m_j < m_lines.size()) 
	{
		m_temp_point++;
		m_out << "\n";
		if (Let())
			continue;
		if (Rem())
			continue;
		if (If())
			continue;
		if (Gosub())
			continue;
		if (For())
			continue;
		if (Next())
			continue;
		if (Return())
			continue;
		if (End())
			break;
		if (Goto())
			continue;
	}
	m_variables.clear();
	m_inserts.clear();
	m_out.close();
}

std::vector<std::vector<std::string>> CodeGenerator::Run(std::vector<Token> lexem)
{
	return std::vector<std::vector<std::string>>();
}


bool CodeGenerator::Let()
{
	if (m_lines[m_j][1] == "LET") 
	{
		m_inserts.clear();
		m_out << "@@Label_" << (stoi(m_lines[m_j][0]) + 1) << ":\n";

		if (m_variables.size() == 0)
			m_variables.push_back({ m_lines[m_j][2], 0 });
		else 
		{
			for (m_var_num = 0; m_var_num < m_variables.size(); m_var_num++)
				if (m_variables[m_var_num].first == m_lines[m_j][2]) { m_flag = true;	break; }
			for (m_i = 4; m_i < m_lines[m_j].size(); m_i++)
				for (int k = 0; k < m_variables.size(); k++)
					if (m_lines[m_j][m_i] == m_variables[k].first)
					{
						m_lines[m_j].erase(m_lines[m_j].begin() + m_i);
						m_lines[m_j].insert(m_lines[m_j].begin() + m_i, "R" + std::to_string(m_variables[k].second));
						break;
					}
			if (!m_flag) m_variables.push_back({ m_lines[m_j][2], m_variables.back().second + 1 });
		}
		if (m_lines[m_j].size() > 5) 
		{
			LetGenVars();
			LetGenExp();
		}
		else
			if (m_flag)
				m_out << "\tLD R" << m_variables[m_var_num].second << " " << m_lines[m_j][4] << "\n";
			else 
			{
				m_out << "\tLD R" << m_variables.back().second << " " << m_lines[m_j][4] << "\n";
			}
		m_j++;
		return true;
	}
	return false;
}

void CodeGenerator::LetGenVars()
{
	m_i = 0;
	std::vector <std::string> in_exp = m_lines[m_j];
	in_exp.erase(in_exp.begin(), in_exp.begin() + 4);
	while (true) 
	{
		while ((m_i < in_exp.size()) and (in_exp[m_i] != ")"))
			m_i++;
		if (m_i == in_exp.size()) 
		{
			m_i = 0;
			break;
		}
		else 
		{
			int j = m_i - 1;
			std::vector<std::string> exp;
			while ((j >= 0) and (in_exp[j] != "(")) 
			{
				exp.insert(exp.begin(), in_exp[j]);
				j--;
			}
			in_exp.erase(in_exp.begin() + j, in_exp.begin() + m_i + 1);
			if (m_inserts.size() == 0)
				m_inserts.push_back({ exp, 1 });
			else
				m_inserts.push_back({ exp, m_inserts.back().second + 1 });
			in_exp.insert(in_exp.begin() + j, "RL1" + std::to_string(m_inserts.back().second));
			m_i = 0;
			continue;
		}
		break;
	}
	while (true) 
	{
		m_i = 0;
		int in_num = -1;
		while (m_i < m_inserts.size()) 
		{
			if (m_inserts[m_i].first.size() <= 3) { m_i++; continue; }

			bool check = false;
			int k = 0;
			while (k < m_inserts[m_i].first.size()) 
			{
				if ((m_inserts[m_i].first[k] == "*") or (m_inserts[m_i].first[k] == "/") or (m_inserts[m_i].first[k] == "^")) 
				{
					std::vector<std::string> help;
					help.push_back(m_inserts[m_i].first[k - 1]); help.push_back(m_inserts[m_i].first[k]); help.push_back(m_inserts[m_i].first[k + 1]);
					m_inserts.insert(m_inserts.begin() + m_i, { help, in_num });
					in_num--;
					m_i++;
					m_inserts[m_i].first.erase(m_inserts[m_i].first.begin() + (k - 1), m_inserts[m_i].first.begin() + (k + 2));
					m_inserts[m_i].first.insert(m_inserts[m_i].first.begin() + (k - 1), "R01" + std::to_string(-1 * m_inserts[m_i - 1].second));
					check = true;
				}
				k++;
			}
			if (!check)
				m_i++;
		}
		m_i = 0;
		while (m_i < in_exp.size()) 
		{
			if ((in_exp[m_i] == "*") or (in_exp[m_i] == "/") or (in_exp[m_i] == "^")) 
			{
				std::vector<std::string> help;
				help.push_back(in_exp[m_i - 1]); help.push_back(in_exp[m_i]); help.push_back(in_exp[m_i + 1]);
				if (m_inserts.size() > 0)
					m_inserts.push_back({ help, m_inserts.back().second + 1 });
				else
					m_inserts.push_back({ help,  1 });
				in_exp.erase(in_exp.begin() + (m_i - 1), in_exp.begin() + (m_i + 2));
				in_exp.insert(in_exp.begin() + (m_i - 1), "RL1" + std::to_string(m_inserts.back().second));
				continue;
			}
			m_i++;
		}
		m_i = 0;
		while (m_i < m_inserts.size()) 
		{
			if (m_inserts[m_i].first.size() < 4) { m_i++; continue; }

			while ((m_inserts[m_i].first.size() > 3)) 
			{
				std::vector<std::string> help;
				help.push_back(m_inserts[m_i].first[0]); help.push_back(m_inserts[m_i].first[1]); help.push_back(m_inserts[m_i].first[2]);
				m_inserts.insert(m_inserts.begin() + m_i, { help, in_num });
				in_num--;
				m_i++;
				m_inserts[m_i].first.erase(m_inserts[m_i].first.begin(), m_inserts[m_i].first.begin() + 3);
				m_inserts[m_i].first.insert(m_inserts[m_i].first.begin(), "R01" + std::to_string(-1 * m_inserts[m_i - 1].second));
			}
		}
		m_i = 0;
		while (in_exp.size() > 3) 
		{
			std::vector<std::string> help;
			help.push_back(in_exp[0]); help.push_back(in_exp[1]); help.push_back(in_exp[2]);
			m_inserts.push_back({ help, m_inserts.back().second + 1 });
			in_exp.erase(in_exp.begin(), in_exp.begin() + 3);
			in_exp.insert(in_exp.begin(), "R1" + std::to_string(m_inserts.back().second));
		}
		if (in_exp.size() > 1)
			m_inserts.push_back({ in_exp, 0 });
		in_exp.clear();
		break;
	}
}

void CodeGenerator::LetGenExp()
{
	for (int k = 0; k < m_inserts.size() - 1; k++) 
	{
		if (m_inserts[k].first[1] == "+") 
		{
			if (m_inserts[k].second > 0)
				m_out << "\tAdd RL1" + std::to_string(m_inserts[k].second) << " " << m_inserts[k].first[0] << " " << m_inserts[k].first[2] << "\n";
			else
				m_out << "\tAdd R01" + std::to_string(-1 * m_inserts[k].second) << " " << m_inserts[k].first[0] << " " << m_inserts[k].first[2] << "\n";
			continue;
		}
		if (m_inserts[k].first[1] == "-") 
		{
			if (m_inserts[k].second > 0)
				m_out << "\tSub RL1" + std::to_string(m_inserts[k].second) << " " << m_inserts[k].first[0] << " " << m_inserts[k].first[2] << "\n";
			else
				m_out << "\tSub R01" + std::to_string(-1 * m_inserts[k].second) << " " << m_inserts[k].first[0] << " " << m_inserts[k].first[2] << "\n";
			continue;
		}
		if (m_inserts[k].first[1] == "*") 
		{
			if (m_inserts[k].second > 0)
				m_out << "\tMul RL1" + std::to_string(m_inserts[k].second) << " " << m_inserts[k].first[0] << " " << m_inserts[k].first[2] << "\n";
			else
				m_out << "\tMul R01" + std::to_string(-1 * m_inserts[k].second) << " " << m_inserts[k].first[0] << " " << m_inserts[k].first[2] << "\n";
			continue;
		}
		if (m_inserts[k].first[1] == "/") 
		{
			if (m_inserts[k].second > 0)
				m_out << "\tDiv RL1" + std::to_string(m_inserts[k].second) << " " << m_inserts[k].first[0] << " " << m_inserts[k].first[2] << "\n";
			else
				m_out << "\tDiv R01" + std::to_string(-1 * m_inserts[k].second) << " " << m_inserts[k].first[0] << " " << m_inserts[k].first[2] << "\n";
			continue;
		}
		if (m_inserts[k].first[1] == "^") 
		{
			if (m_inserts[k].second > 0)
				m_out << "\tPow RL1" + std::to_string(m_inserts[k].second) << " " << m_inserts[k].first[0] << " " << m_inserts[k].first[2] << "\n";
			else
				m_out << "\tPow R01" + std::to_string(-1 * m_inserts[k].second) << " " << m_inserts[k].first[0] << " " << m_inserts[k].first[2] << "\n";
			continue;
		}
	}
	if (m_inserts.back().first.size() == 1) m_inserts.pop_back();
	if (m_flag)
	{
		if (m_inserts.back().first[1] == "+")
			m_out << "\tAdd R" << m_variables[m_var_num].second << " " << m_inserts.back().first[0] << " " << m_inserts.back().first[2] << "\n";
		if (m_inserts.back().first[1] == "-")
			m_out << "\tSub R" << m_variables[m_var_num].second << " " << m_inserts.back().first[0] << " " << m_inserts.back().first[2] << "\n";
		if (m_inserts.back().first[1] == "*")
			m_out << "\tMul R" << m_variables[m_var_num].second << " " << m_inserts.back().first[0] << " " << m_inserts.back().first[2] << "\n";
		if (m_inserts.back().first[1] == "/")
			m_out << "\tDiv R" << m_variables[m_var_num].second << " " << m_inserts.back().first[0] << " " << m_inserts.back().first[2] << "\n";
		if (m_inserts.back().first[1] == "^")
			m_out << "\tPow R" << m_variables[m_var_num].second << " " << m_inserts.back().first[0] << " " << m_inserts.back().first[2] << "\n";
	}
	else
	{
		if (m_inserts.back().first[1] == "+")
			m_out << "\tAdd R" << m_variables.back().second << " " << m_inserts.back().first[0] << " " << m_inserts.back().first[2] << "\n";
		if (m_inserts.back().first[1] == "-")
			m_out << "\tSub R" << m_variables.back().second << " " << m_inserts.back().first[0] << " " << m_inserts.back().first[2] << "\n";
		if (m_inserts.back().first[1] == "*")
			m_out << "\tMul R" << m_variables.back().second << " " << m_inserts.back().first[0] << " " << m_inserts.back().first[2] << "\n";
		if (m_inserts.back().first[1] == "/")
			m_out << "\tDiv R" << m_variables.back().second << " " << m_inserts.back().first[0] << " " << m_inserts.back().first[2] << "\n";
		if (m_inserts.back().first[1] == "^")
			m_out << "\tPow R" << m_variables.back().second << " " << m_inserts.back().first[0] << " " << m_inserts.back().first[2] << "\n";
	}
}


bool CodeGenerator::Rem()
{
	if (m_lines[m_j][1] == "REM") {
		m_out << "@@Label_" << (stoi(m_lines[m_j][0]) + 1) << ":\n";
		m_out << "\t\t\t\t\t;";
		for (int i = 2; i < m_lines[m_j].size(); i++)
			m_out << m_lines[m_j][i];
		m_out << "\n";
		m_temp_point++;
		m_j++;
		return true;
	}
	return false;
}

bool CodeGenerator::Gosub()
{
	if (m_lines[m_j][1] == "GOSUB") 
	{
		m_out << "@@Label_" << (stoi(m_lines[m_j][0]) + 1) << ":\n";
		m_out << "\tST 10, #" << m_temp_point << "\n";
		m_out << "\tjmp @@Label_" << stoi(m_lines[m_j][2]) + 1 << "\n";
		m_j++;
		return true;
	}
	return false;
}

bool CodeGenerator::For()
{
	if (m_lines[m_j][1] == "FOR") 
	{
		if (m_variables.size() == 0)
			m_variables.push_back({ m_lines[m_j][2], 0 });
		else {
			bool flag = false;
			for (m_var_num = 0; m_var_num < m_variables.size(); m_var_num++)
				if (m_variables[m_var_num].first == m_lines[m_j][2]) { flag = true;	break; }
			if (!flag)
				m_variables.push_back({ m_lines[m_j][2], m_variables.back().second + 1 });
		}
		m_out << "\tLD R" << m_variables[m_var_num].second << " " << m_lines[m_j][4] << "\n";
		int k = 0;
		m_out << "@@Label_" << (stoi(m_lines[m_j][0]) + 1) << ":\n";
		m_out << "\tLD R00001 " << m_lines[m_j][6] << "\n";
		while ((m_j < m_lines.size()) and (m_lines[m_j][1] != "NEXT")) 
		{
			m_j++;
			k++;
		}
		m_out << "\tSub R02 R00001 R" << m_variables.back().second << "\n";
		m_out << "\tBLTZ R02 @@Label_" << stoi(m_lines[m_j][0]) + 2 << "\n";
		m_j -= k;
		m_j++;
		return true;
	}
	return false;
}

bool CodeGenerator::Next()
{
	if (m_lines[m_j][1] == "NEXT") 
	{
		m_out << "@@Label_" << (stoi(m_lines[m_j][0]) + 1) << ":\n";
		int k = 0;
		m_temp_point++;
		while ((m_j > 0) and (m_lines[m_j][1] != "FOR")) 
		{
			m_j--;
			k++;
		}
		int t = m_variables.size() - 1;
		while (m_variables[t].first != m_lines[m_j][2])
			t--;
		if (m_lines[m_j].size() > 7) 
		{
			m_out << "\tAdd R" << m_variables[t].second << " " << m_lines[m_j][8] << "\n";
			m_out << "\tjmp @@Label_" << stoi(m_lines[m_j][0]) + 1 << '\n';
		}
		else {
			m_out << "\tinc R" << m_variables[t].second << "\n";
			m_out << "\tjmp @@Label_" << stoi(m_lines[m_j][0]) + 1 << '\n';
		}
		m_j += k;
		m_j++;
		return true;
	}
	return false;
}

bool CodeGenerator::Return()
{
	if (m_lines[m_j][1] == "RETURN") 
	{
		m_out << "@@Label_" << (stoi(m_lines[m_j][0]) + 1) << ":\n";
		m_out << "\tBR *10\n";
		m_temp_point++;
		m_j++;
		return true;
	}
	return false;
}

bool CodeGenerator::End()
{
	if (m_lines[m_j][1] == "END") 
	{
		m_out << "@@Label_" << (stoi(m_lines[m_j][0]) + 1) << ":\n";
		m_out << "\thalt\n";
		m_out << "END Start";
		return true;
	}
	return false;
}

bool CodeGenerator::Goto()
{
	if (m_lines[m_j][1] == "GOTO") 
	{
		m_out << "@@Label_" << (stoi(m_lines[m_j][0]) + 1) << ":\n";
		m_out << "\tjmp @@Label_" << m_lines[m_j][2] << "\n";
		m_temp_point++;
		m_j++;
		return true;
	}
	return false;
}
