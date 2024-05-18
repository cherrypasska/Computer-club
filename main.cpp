#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <iomanip>
#include <string>

struct Client{
	std::string name;
	int table = -1;
	std::string arrival_time;
	std::string start_time;
};

struct Table{
	int number;
	int revenue = 0;
	int occupied_minutes = 0;
	bool is_occupied = false;
	std::string occupied_start_time;
};

std::vector<std::string> split(const std::string& str, char delimiter){
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);
	while(std::getline(tokenStream, token, delimiter)){
		tokens.push_back(token);
	}
	return tokens;
}

int time_to_minutes(const std::string& time){
	auto parts = split(time, ':');
	int hours = std::stoi(parts[0]);
	int minutes = std::stoi(parts[1]);
	return hours * 60 + minutes;
}

std::string minutes_to_time(int minutes){
	int hours = minutes / 60;
	minutes %= 60;
	std::ostringstream oss;
	oss << std::setw(2) << std::setfill('0') << hours << ":"
		<< std::setw(2) << std::setfill('0') << minutes;
	return oss.str();
}

void print_event(const std::string& time, const std::string& event){
	std::cout << time << " " << event << "\n";
}

int main(int argc, char* argv[]){
	if(argc < 2){
		std::cerr << "Usage: " << argv[0] << " <input_file>\n";
		return 1;
	}
	std::ifstream input_file(argv[1]);
	if(!input_file){
		std::cerr << "Error opening file: " << argv[1] << "\n";
		return 1;
	}

	std::string line;
	std::getline(input_file, line);
	int table_count = std::stoi(line);
	std::getline(input_file, line);
	auto work_hours = split(line, ' ');
	std::string open_time = work_hours[0];
	std::string close_time = work_hours[1];
	std::getline(input_file, line);
	int hour_cost = std::stoi(line);

	std::map<std::string, Client> clients;
	std::vector<Table> tables(table_count);
	for(int i = 0; i < table_count; ++i){
		tables[i].number = i + 1;
	}
	std::queue<std::string> waiting_queue;

	print_event(open_time, "");

	while(std::getline(input_file, line)){
		auto parts = split(line, ' ');
		std::string event_time = parts[0];
		int event_id = std::stoi(parts[1]);

		if(event_time < open_time){
			std::string client_name = parts[2];
			print_event(event_time, "1 " + client_name);
			print_event(event_time, "13 NotOpenYet");
			continue;
		}

       		switch(event_id){
        		case 1:{
            			std::string client_name = parts[2];
            			if(clients.find(client_name) != clients.end()){
					print_event(event_time, "1 " + client_name);
                			print_event(event_time, "13 YouShallNotPass");
            			}
				else{
                			clients[client_name] = { client_name, -1, event_time, "" };
                			print_event(event_time, "1 " + client_name);
            			}
            			break;
        		}
        		case 2:{
           			std::string client_name = parts[2];
           	 		int table_number = std::stoi(parts[3]);
            			if(clients.find(client_name) == clients.end()){
					print_event(event_time, "2 " + client_name + " " + std::to_string(table_number));
                			print_event(event_time, "13 ClientUnknown");
            			}
				else{
                			Client& client = clients[client_name];
                			if(table_number < 1 || table_number > table_count || tables[table_number - 1].is_occupied){
						print_event(event_time, "2 " + client_name + " " + std::to_string(table_number));
                    				print_event(event_time, "13 PlaceIsBusy");
                			}
					else{
                    				if(client.table != -1){
                        				tables[client.table - 1].is_occupied = false;
                    				}
                    				client.table = table_number;
                    				tables[table_number - 1].is_occupied = true;
                    				tables[table_number - 1].occupied_start_time = event_time;
                    				print_event(event_time, "2 " + client_name + " " + std::to_string(table_number));
                			}
            			}
            			break;
        		}
        		case 3:{
            			std::string client_name = parts[2];
            			if(clients.find(client_name) == clients.end()){
                			print_event(event_time, "13 ClientUnknown");
            			}
				else{
                			bool has_free_table = false;
                			for(const auto& table : tables){
                    				if(!table.is_occupied){
                        				has_free_table = true;
                        				break;
                    				}
                			}
                			if(has_free_table){
						print_event(event_time, "3 " + client_name);
                    				print_event(event_time, "13 ICanWaitNoLonger!");
                			}
					else{
                    				if(waiting_queue.size() >= table_count){
							print_event(event_time, "3 " + client_name);
                        				print_event(event_time, "11 " + client_name);
                    				}
						else{
                        				waiting_queue.push(client_name);
                        				print_event(event_time, "3 " + client_name);
                    				}
                			}
            			}
            			break;
        		}
        		case 4:{
            			std::string client_name = parts[2];
            			if(clients.find(client_name) == clients.end() || clients[client_name].table == -1) {
					print_event(event_time, "4 " + client_name);
                			print_event(event_time, "13 ClientUnknown");
            			}
				else{
                			Client& client = clients[client_name];
                			Table& table = tables[client.table - 1];
                			int start_time = time_to_minutes(table.occupied_start_time);
                			int end_time = time_to_minutes(event_time);
                			int occupied_minutes = end_time - start_time;
                			table.occupied_minutes += occupied_minutes;
                			table.revenue += (occupied_minutes + 59) / 60 * hour_cost;
                			table.is_occupied = false;
                			client.table = -1;
                			print_event(event_time, "4 " + client_name);
                			if(!waiting_queue.empty()){
                    				std::string next_client_name = waiting_queue.front();
                    				waiting_queue.pop();
                    				clients[next_client_name].table = table.number;
                    				table.is_occupied = true;
                    				table.occupied_start_time = event_time;
                    				print_event(event_time, "12 " + next_client_name + " " + std::to_string(table.number));
					}
				}
				break;
			}
			default:
				break;
		}
	}

	for(const auto& client_pair : clients){
		const auto& client = client_pair.second;
			if(client.table != -1){
			Table& table = tables[client.table - 1];
			int start_time = time_to_minutes(table.occupied_start_time);
			int end_time = time_to_minutes(close_time);
			int occupied_minutes = end_time - start_time;
			table.occupied_minutes += occupied_minutes;
			table.revenue += (occupied_minutes + 59) / 60 * hour_cost;
			print_event(close_time, "11 " + client.name);
		}
	}

	print_event(close_time, "");

	for(const auto& table : tables){
		std::cout << table.number << " " << table.revenue << " " << minutes_to_time(table.occupied_minutes) << "\n";
	}

	return 0;
}
