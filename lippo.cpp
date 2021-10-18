/*                                                                      
  @@@@@@@@@@@                                       @@@@@@@@@@@         
  @@@@@@@@@@@@@@@@@                           @@@@@@@@@@@@@@@@@         
   @@@@@@@@@@@@@@@@@@    ##########,,,##   @@@@@@@@@@@@@@@@@@           
     @@@@@@@@@@@@@@@@@@  %%%%%%%%%%%%%((  @@@@@@@@@@@@@@@@@@            
       @@@@@@@@@@@@@@@@@%%%%%%%%%%%%%%((%@@@@@@@@@@@@@@@@@              
          @@@@@@@@@@@@@@%%%%%%%%%%%%%%%(%@@@@@@@@@@@@@@                 
            @@@@@@@@@@@ %%%%%%%%%%%%%%%(@@@@@@@@@@@@@                   
               @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                      
                @@@@@@@▆▆▆▆▆@@@@@@@@@▆▆▆▆▆@@@@@@@                       
               @@@@@▆▆▆▆▆▆▆▆▆▆▆@@@▆▆▆▆▆▆▆▆▆▆▆@@@@                       
               @@@@#▆▆▆▆▆▆▆▆▆▆▆▆@▆▆▆▆▆▆▆▆▆▆▆▆@@@@                       
               @@@@@▆▆▆▆▆▆▆▆▆▆▆@@@▆▆▆▆▆▆▆▆▆▆▆@@@@                       
                @@@@@,▆▆▆▆▆▆▆▆@@@@@▆▆▆▆▆▆▆▆@@@@@                        
                @@@@@@@@@@@@@@@@ @@@@@@@@@@@@@@@                        
                 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                         
                   @@@@@@@@@@@@@@@@@@@@@@@@@@@                          
                    ,@@@@@@@@@ @@@%@@@@@@@@@,                           
                  ,,,,,,@@@@@@@@@@@@@@@@@@,,,,,,                        
                 ,,,,,,,@@@@@@@@@@@@@@@@@@,,,,,,                        
                   ,,,,,,,,,,,,@@@@,,,,,,,,,((                          
                      ((/,,,,,,,,,,,,,,((((%                            
                       %%%%(((((((((((((%%%                             
                      %%%%%%%%%%%%%%%%%%%%%                             
                      %%%%%%%%%%%%%%%%%%%%%%                            
                     %%%%%%*,%,%%%%%%%%%%/%%%                           
===================================================
===================================================
THIS IS A PROFESSIONAL BOT.
THEIR NAME IS LIPPO, I GUESS???

AND NOW THEY'RE WRITTEN IN C++! I DON'T KNOW IF
THIS MAKES ME MORE OR LESS OF A MASOCHIST THAN
USING JAVASCRIPT. PROBABLY SLIGHTLY LESS.

Written by Diane Sparks for the VGDC @ UCI server
Uses D++ by Craig Edwards
Art generated on manytools.org
===================================================
===================================================
*/

#include <dpp/dpp.h>

#include <iostream>
#include <regex>
#include <fstream>
#include <algorithm>

namespace {
	bool lab_open = false;

	constexpr dpp::snowflake VgdcServerId = 228326116270538753;
	constexpr dpp::snowflake RoleOperations = 364492667335213057;
	constexpr dpp::snowflake RoleAdmin = 230588792820596739;
	constexpr dpp::snowflake ChannelLabStatus = 629369478462963722;

	constexpr const char* TokenFile = "token/token.txt";

	std::regex secret_lab_regex{R"reg((?:[s$]\s*(?:[e3 ]\s*)+[ck]\s*[r4]\s*(?:[e3 i1]\s*)+[t7]\s*([e3 ]\s*)*\s*[l1]\s*[a@8 ]\s*[b8]))reg", std::regex_constants::icase};
}

void set_lab_open(dpp::cluster& bot, bool open) {
	lab_open = open;

	dpp::presence presence{
		open ? dpp::ps_online : dpp::ps_dnd,
		dpp::activity{
			dpp::at_game,
			open ? std::string("Game Lab OPEN") : std::string("Game Lab CLOSED"),
			open ? std::string("OPEN") : std::string("CLOSED"),
			std::string()
		}
	};

	bot.set_presence(presence);
}


std::string mention(dpp::snowflake user_id) {
	return "<@" + std::to_string(user_id) + ">";
}


int main() {
	std::ifstream token_file{TokenFile};
	if (!token_file.is_open()) {
		std::cerr << "Failed to open token file!" << std::endl;
		return 1;
	}

	std::string token;
	token_file >> token;
	token_file.close();

	dpp::cluster bot{token};

	bot.on_ready([&bot] (const dpp::ready_t  event) {
		std::cout << "Connected as " << bot.me.username << std::endl;

		set_lab_open(bot, false);
	});

	bot.on_message_create([&bot] (const dpp::message_create_t  event) {
		if (event.msg->author->id == bot.me.id) {
			return;
		}

		if (event.msg->guild_id == VgdcServerId && std::regex_search(event.msg->content, secret_lab_regex)) {
			bot.message_create(dpp::message(event.msg->channel_id, mention(event.msg->author->id) + "I think you mean \"Quiet Lab.\""));
			return;
		}

		if (event.msg->content == "!labopen" && event.msg->channel_id == ChannelLabStatus) {
			for (auto  role : event.msg->member.roles) {
				if (role == RoleOperations || role == RoleAdmin) {
					set_lab_open(bot, true);
					bot.message_create(dpp::message(event.msg->channel_id, "Game lab is now OPEN"));
					return;
				}
			}
		}

		if ((event.msg->content == "!labclose" || event.msg->content == "!labclosed") && event.msg->channel_id == ChannelLabStatus) {
			for (auto  role : event.msg->member.roles) {
				if (role == RoleOperations || role == RoleAdmin) {
					set_lab_open(bot, false);
					bot.message_create(dpp::message(event.msg->channel_id, "Game lab is now CLOSED"));
					return;
				}
			}
		}
	});

	bot.start(false);
}
