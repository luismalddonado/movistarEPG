#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "inih/ini.h"
#include <curl/curl.h>
#include <glib.h>


#define line_length 5000
#define min_line_length 50
#define MAX_CHANNELS 350
#define channel_length 30
#define date_length 11
#define time_length 9
#define url_pamateres_length 5000


struct  EPGConfiguration
{
    char* copy_to_hd;
    char* output_unzipped_file;
    char* channel_config_file;
	char* GMT;
	char* URL;
	char* URL_parameters;
	char* channels_list;
	char* downloaded_file;
	int	  days_to_download;
};


struct program
{
   char  start_date[date_length];
   char  start_time[time_length];
   char  end_date[date_length];
   char  end_time[time_length];
   char  *title;
   char	 *description; 
   unsigned long long int theTimeStamp;
};

struct channel
{
   char  channel_name[channel_length];
   GList* programlist; 
};

typedef struct channelConfig
{
   char*  channel_name;
} channelConfig;

struct channelHD
{
   char  channel_name[channel_length];
   int	 SD_channel;
};

EPGConfiguration theEPGConfiguration;


int generateInputFile()
{
	
	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);

	
	time_t rawtime;
    struct tm * timeinfo;

	
	char *start_date = (char *)malloc(25 * sizeof(char));
	char *end_date = (char *)malloc(25 * sizeof(char));
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    sprintf(start_date, "%d-%02d-%02d",timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,timeinfo->tm_mday);
	
	timeinfo->tm_mday += theEPGConfiguration.days_to_download;
	mktime(timeinfo);
	sprintf(end_date, "%d-%02d-%02d", timeinfo->tm_year + 1900,timeinfo->tm_mon + 1,timeinfo->tm_mday);
	
	char url_parameters[url_pamateres_length];
	
	url_parameters[0]='\0';
	
	
	strcat(url_parameters, theEPGConfiguration.URL_parameters);
	strcat(url_parameters, "&export-date-from=");
	strcat(url_parameters, start_date);
	strcat(url_parameters, "&export-date-to=");
	strcat(url_parameters, end_date);
	strcat(url_parameters, "&");
	strcat(url_parameters, theEPGConfiguration.channels_list);
	
	
	printf("Start date: %s\n",start_date);
	printf("Days to download: %d. End date: %s\n",theEPGConfiguration.days_to_download,end_date);

	printf("Sending 'POST' request to URL : %s\n", theEPGConfiguration.URL);
	printf("Post parameters : %s \n",url_parameters);

	printf("Starting getting EPG data from the web.\n");
	
	curl = curl_easy_init();
	if(curl) {
	  
		FILE *fp;  

		fp = fopen(theEPGConfiguration.downloaded_file,"wb");

		curl_easy_setopt(curl, CURLOPT_URL, theEPGConfiguration.URL);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, url_parameters);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

		res = curl_easy_perform(curl);

		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		curl_easy_cleanup(curl);
		
		fflush(fp);
		
		fclose(fp);
		
	}
	free(start_date);
	free(end_date);
	curl_global_cleanup();
	
	printf("Finished getting EPG data from the web.\n");
	
	return 0;
}

int findSubstring(const char *str,const char *sub)
{

  const char *p1, *p2, *p3;

  unsigned i=0,j=0;

  p1 = str;
  p2 = sub;
  
  for(i = 0; i<strlen(str); i++)
  {
    if(*p1 == *p2)
      {
          p3 = p1;
          for(j = 0;j<strlen(sub);j++)
          {
            if(*p3 == *p2)
            {
              p3++;p2++;
            } 
            else
              break;
          }
          p2 = sub;
          if(j == strlen(sub))
          {
             return i;
          }
      }
    p1++; 
  }
  
  return -1;
  
 }
 
static int configurationHandler(void* user, const char* section, const char* name, const char* value)
{
    EPGConfiguration* pconfig = (EPGConfiguration*)user;

	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("channels", "copy_to_hd")) {
        pconfig->copy_to_hd = strdup(value);
		printf(" copy_to_hd: %s\n", pconfig->copy_to_hd);
    } else if (MATCH("epgconfiguration", "output_unzipped_file")) {
        pconfig->output_unzipped_file = strdup(value);
		printf(" output_unzipped_file: %s\n", pconfig->output_unzipped_file);
    } else if (MATCH("epgconfiguration", "channel_config_file")) {
        pconfig->channel_config_file = strdup(value);
		printf(" channel_config_file: %s\n", pconfig->channel_config_file);
	} else if (MATCH("epgconfiguration", "GMT")) {
        pconfig->GMT = strdup(value);
		printf(" GMT: %s\n", pconfig->GMT);
    } else if (MATCH("movistardata", "URL")) {
        pconfig->URL = strdup(value);
		printf(" URL: %s\n", pconfig->URL);
    } else if (MATCH("movistardata", "URL_parameters")) {
        pconfig->URL_parameters = strdup(value);
		printf(" URL_parameters: %s\n", pconfig->URL_parameters);
    } else if (MATCH("movistardata", "channels_list")) {
        pconfig->channels_list = strdup(value);
		printf(" channels_list: %s\n", pconfig->channels_list);
    } else if (MATCH("movistardata", "downloaded_file")) {
        pconfig->downloaded_file = strdup(value);
		printf(" downloaded_file: %s\n", pconfig->downloaded_file);
    } else if (MATCH("movistardata", "days_to_download")) {
        pconfig->days_to_download = atoi(value);
		printf(" days_to_download: %d\n", pconfig->days_to_download);
	}else {
        return 0;  
    }
    return 1;
}

int initConfig()
{
    if (ini_parse("../properties/movistarEPG.ini", configurationHandler, &theEPGConfiguration) < 0) {
        printf("Can't load 'movistarEPG.ini'\n");
        return -1;
    }
	
	return 0;
}



void calculateEndTime(channel * theChannel)
{


	GList *elem;
	for(elem = theChannel->programlist; elem; elem = elem->next) 
	{
		program * theCurrentProgram;
		theCurrentProgram= (program*)elem->data;
		
		if (elem -> next != NULL)
		{
			program * theNextProgram;
			theNextProgram= (program*)(elem->next)->data;
		
			memcpy(theCurrentProgram -> end_date,theNextProgram -> start_date,date_length);
			memcpy(theCurrentProgram -> end_time,theNextProgram -> start_time,time_length);
		}
		else
		{
			memcpy(theCurrentProgram -> end_date,theCurrentProgram-> start_date,date_length);
			memcpy(theCurrentProgram -> end_time,theCurrentProgram-> start_time,time_length);
		}
	}
	
		
}

void printtheChannel(channel *theChannel)
{
	printf("Channel: *%s*\n", theChannel->channel_name);
	printf("Channels size: *%d*\n", g_list_length(theChannel->programlist));
	
	int i=0;
	GList *elem;
	for(elem = theChannel->programlist; elem; elem = elem->next) 
	{
		program * theCurrentProgram;
		theCurrentProgram= (program*)elem->data;
		i++;
		printf("	Program: at position %i: *%s*\n", i, theCurrentProgram -> title);
		printf("		TimeStamp *%llu*\n", theCurrentProgram -> theTimeStamp);
		printf("		Start Date *%s*\n", theCurrentProgram -> start_date);
		printf("		Start Time *%s*\n", theCurrentProgram -> start_time);
		printf("		End Date *%s*\n", theCurrentProgram -> end_date);
		printf("		End Time  *%s*\n", theCurrentProgram -> end_time);
	}
	
		
}



void printthePrograma (program *thePrograma)
{

		if (thePrograma -> start_date!=NULL)
			printf("start_date: *%s*\n", thePrograma -> start_date);
		else	
			printf("start_date: NULL\n");
		
		if (thePrograma -> start_time!=NULL)
			printf("start_time: *%s*\n", thePrograma -> start_time);
		else	
			printf("start_time: NULL\n");
	
		if (thePrograma -> end_date!=NULL)
			printf("end_date: *%s*\n", thePrograma -> end_date);
		else	
			printf("end_date: NULL\n");
	
		if (thePrograma -> end_time!=NULL)
			printf("end_time: *%s*\n", thePrograma -> end_time);
		else	
			printf("end_time: NULL\n");
	
		if (thePrograma -> title!=NULL)
			printf("title: *%s*\n", thePrograma -> title);
		else	
			printf("title: NULL\n");
		
		if (thePrograma -> description!=NULL)
			printf("description: *%s*\n", thePrograma -> description);
		else	
			printf("description: NULL\n");
		

		printf("theTimeStamp: *%llu*\n", thePrograma -> theTimeStamp);

}
                                                                                                    

char * getTime(){
    time_t rawtime;
    struct tm * timeinfo;
	
	char *result = (char *)malloc(25 * sizeof(char));
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    sprintf(result, "%02d/%02d/%d %02d:%02d:%02d",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	
	return result;
}


char * normalizeChannelName(char *inputString)
{

	char *result = (char *)malloc(strlen (inputString) * sizeof(char)+1);

	unsigned int i;
	for( i = 0; i < strlen(inputString); i = i + 1 )
	{
		
		switch(inputString[i])
		{
		case '&' :
			result[i]='-';
			break;
		case ' ' :
			result[i]='-';
			break;
		default :
			result[i]=inputString[i];
	   }
	    
		char c=result[i];
		int ascii_code = (int)c;
		if (ascii_code>127 ||  ascii_code<0 )
		{
			result[i]='?'; 
		}
		
		
	}
	result[i]= '\0';
	return result;
}

char * normalizeString(char *inputString)
{
	char *result = (char *)malloc(strlen (inputString) * sizeof(char)+1);

	unsigned int i;
	for( i = 0; i < strlen(inputString); i = i + 1 )
	{
		
		switch(inputString[i])
		{
		case '&' :
			result[i]='-';
			break;
		default :
			result[i]=inputString[i];
	   }
	
		
	}
	result[i]= '\0';
	return result;
}

char * getStartTime (char * date, char * time)
{
		char *result = (char *)malloc(40 * sizeof(char));
		result[0]='\0';
		strncat (result, date, 4 );
		strncat (result, date+5, 2 );
		strncat (result, date+8, 2 );
		
		strncat (result, time, 2 );
		strncat (result, time+3, 2 );
		strncat (result, time+6, 2 );
		
		strncat (result, " ",1);
		
		strncat (result, theEPGConfiguration.GMT,6);
		
		return result;
}

void generateProgramStartDateandTime(program *theNextPrograma)
{
	if ((theNextPrograma -> start_date!= NULL) && (theNextPrograma -> start_time!=NULL))
	{
		char  year[5];
		char  month[3];
		char  day[3];
		char  hour[3];
		char  min[3];
		memcpy ( year, theNextPrograma -> start_date + 2, 2 );
		year[4]='\0';
		memcpy ( month, theNextPrograma -> start_date + 5, 2);
		month[2]='\0';
		memcpy ( day, theNextPrograma -> start_date + 8, 2);
		day[2]='\0';
		memcpy ( hour, theNextPrograma -> start_time + 0, 2);
		hour[2]='\0';
		memcpy ( min, theNextPrograma -> start_time + 3, 2);
		min[2]='\0';
		theNextPrograma -> theTimeStamp =atol (year)*100000000+atol (month)*1000000+atol (day)*10000+atol (hour)*100+atol (min);
	}
	
}
void generateProgramData(const char * thefecha,const char * thestart_time,const char * thetitle,const char * thedescription,program *theNextPrograma)
{


	if (thefecha != NULL)
		strcpy(theNextPrograma -> start_date, thefecha);
	
	if (thestart_time != NULL)
		strcpy(theNextPrograma -> start_time, thestart_time);
	
	if (thetitle != NULL)
	{
		theNextPrograma -> title = (char *) malloc( strlen(thetitle) +1 );
		strcpy(theNextPrograma -> title, thetitle);
	}
	else
	{
		theNextPrograma -> title = (char *) malloc(1*sizeof(char));
		theNextPrograma -> title[0]='\0';
		
	}
	
	if (thedescription != NULL)
	{
		theNextPrograma -> description = (char *) malloc( strlen(thedescription) +1 );
		strcpy(theNextPrograma -> description, thedescription);
	}
	else
	{
		theNextPrograma -> description = (char *) malloc(1*sizeof(char));
		theNextPrograma -> description[0]='\0';
		
	}
	
	generateProgramStartDateandTime(theNextPrograma);
	

}

gint programsComparator (gpointer a, gpointer b)
{
	return (((program *)a)->theTimeStamp)>(((program *)b)->theTimeStamp) ;

}

void printtheChannelConfig(GList* list )
{
	
	GList *elem;
	

	for(elem = list; elem; elem = elem->next) 
	{
	  channelConfig *thechannelConfig;
	  thechannelConfig= (channelConfig*) elem->data;
	  printf("CHANNEL NAME*%s*\n",thechannelConfig ->channel_name);
	}

	
		
}

int isChannelInchannelConfig (char channel[],GList *thechannelConfigList)
{
	
	
	GList *elem;
			
	for(elem = thechannelConfigList; elem; elem = elem->next) 
	{
		
		channelConfig *thechannelConfig;
		thechannelConfig= (channelConfig*) elem->data;
		
		if ( strcmp ( channel, thechannelConfig -> channel_name ) ==0)
		{
				return 1;
		}	
	}
	
	return 0;
	
}



