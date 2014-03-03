/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  Reply codes
 *
 */

#ifndef REPLY_H_
#define REPLY_H_

#define RPL_WELCOME 	"001"
#define RPL_YOURHOST 	"002"
#define RPL_CREATED		"003"
#define RPL_MYINFO		"004"

#define RPL_LUSERCLIENT		"251"
#define RPL_LUSEROP			"252"
#define RPL_LUSERUNKNOWN	"253"
#define RPL_LUSERCHANNELS	"254"
#define RPL_LUSERME			"255"

#define RPL_AWAY			"301"
#define RPL_UNAWAY          "305"
#define RPL_NOWAWAY         "306"

#define RPL_WHOISUSER		"311"
#define RPL_WHOISSERVER		"312"
#define RPL_WHOISOPERATOR		"313"
#define RPL_WHOISIDLE		"317"
#define RPL_ENDOFWHOIS		"318"
#define RPL_WHOISCHANNELS		"319"

#define RPL_WHOREPLY		"352"
#define RPL_ENDOFWHO		"315"

#define RPL_LIST			"322"
#define RPL_LISTEND			"323"

#define RPL_CHANNELMODEIS	"324"

#define RPL_NOTOPIC			"331"
#define RPL_TOPIC			"332"

#define RPL_NAMREPLY		"353"
#define RPL_ENDOFNAMES		"366"

#define RPL_MOTDSTART		"375"
#define RPL_MOTD			"372"
#define RPL_ENDOFMOTD		"376"

#define RPL_YOUREOPER		"381"

#define ERR_NOSUCHNICK			"401"
#define ERR_NOSUCHCHANNEL		"403"
#define ERR_CANNOTSENDTOCHAN	"404"
#define ERR_UNKNOWNCOMMAND		"421"
#define ERR_NOMOTD              "422"
#define ERR_NICKNAMEINUSE		"433"
#define ERR_USERNOTINCHANNEL	"441"
#define ERR_NOTONCHANNEL		"442"
#define ERR_NOTREGISTERED		"451"
#define ERR_ALREADYREGISTRED	"462"
#define ERR_PASSWDMISMATCH      "464"
#define ERR_UNKNOWNMODE			"472"
#define ERR_CHANOPRIVSNEEDED	"482"
#define ERR_UMODEUNKNOWNFLAG	"501"
#define ERR_USERSDONTMATCH		"502"

#include "client.h"
#include "channel.h"

//FUNCTION DECLARATIONS


bstring getFullUserId(cli *client);

// ERR replies
void send_ERR_NICKNAMEINUSE(cli *client, bstring nick);
void send_ERR_NOSUCHNICK(cli *client, bstring nick);
void send_ERR_ALREADYREGISTERED(cli *client);
void send_ERR_CANNOTSENDTOCHAN(cli *client, bstring channelName);
void send_ERR_NOTONCHANNEL(cli *client, bstring channelName);
void send_ERR_NOSUCHCHANNEL(cli *client, bstring channelName);
void send_ERR_PASSWDMISMATCH(cli *client);
void send_ERR_UMODEUNKOWNFLAG(cli *client);
void send_ERR_USERSDONTMATCH(cli *client);
void send_ERR_CHANOPRIVSNEEDED(cli *client, bstring channelName);
void send_ERR_UNKOWNMODE(cli *client, char modeChar, bstring channelName);
void send_ERR_USERNOTINCHANNEL(cli *client, bstring nick, bstring channelName);

// RPL replies

void send_RPL_WELCOME(cli *client);
void send_RPL_YOURHOST(cli *client);
void send_RPL_CREATED(cli *client);
void send_RPL_MYINFO(cli *client);
void send_ERR_NOMOTD(cli *client);
void send_RPL_LUSERCLIENT(cli *client, int numClients);
void send_RPL_LUSEROP(cli *client);
void send_RPL_LUSERUNKNOWN(cli *client, int numUnknown);
void send_RPL_LUSERME(cli *client, int numConnections);
void send_RPL_LUSERCHANNELS(cli *client);
void send_RPL_WHOISUSER(cli *sender, cli *queried);
void send_RPL_WHOISSERVER(cli *sender, cli *queried);
void send_RPL_ENDOFWHOIS(cli *sender, cli *queried);
void send_RPL_NAMREPLY(cli *client, chnl *channel);
void send_RPL_ENDOFNAMES(cli *client, chnl *channel);
void send_RPL_NOTOPIC(cli *client, bstring channelName);
void send_RPL_TOPIC(cli *client, bstring channelName, bstring channelTopic);
void send_RPL_YOUREOPER(cli *client);
void send_RPL_CHANNELMODEIS(cli *client, bstring channelName, bstring channelMode);
void send_RPL_UNAWAY(cli *client);
void send_RPL_NOWAWAY(cli *client);
void send_RPL_AWAY(cli *client, bstring nick, bstring message);

#endif /* REPLY_H_ */
