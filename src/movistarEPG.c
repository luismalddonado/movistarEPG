#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <glib.h>
#include "movistarEPG.h"

channel  channel_list_SD[MAX_CHANNELS];
channelHD  channel_list_HD[MAX_CHANNELS];
int last_SD_channel_index;
int last_HD_channel_index;


GList *thechannelConfigList;

int returnSDchannel (char channel[]);
int loadChannelsConfigData();
int retreiveSDDatainXML();
int retreiveHDData();
int generateData();
int checkData();
int sortSDChannels();
int calculateSDChannelsendtime();
int retreiveSDDatainTXT();
void addProgramm(char * thecadena,program * theNextPrograma);

int main () {
	
	
	int i;
    i=initConfig();
	if (i<0)
	{
		printf("Error during initConfig()");
		return -1;
	}


	i=generateInputFile();
	if (i<0)
	{
		printf("Error during generateInputFile()");
		return -1;
	}


	i=loadChannelsConfigData();
	if (i<0)
	{
		printf("Error during loadChannelsConfigData()");
		return -1;
	}

	if (strstr  (theEPGConfiguration.URL_parameters, "format=xml")!=NULL)
	{
		printf("Getting data in XML format\n");
		i=retreiveSDDatainXML();
		if (i<0)
		{
			printf("Error during retreiveSDDatainXML()");
			return -1;
		}
	}
	else if (strstr  (theEPGConfiguration.URL_parameters, "format=txt")!=NULL)
	{
		printf("Getting data in TXT format\n");
		i=retreiveSDDatainTXT();
		if (i<0)
		{
			printf("Error during retreiveSDDatainXML()");
			return -1;
		}
		
	}
	else
	{
		printf("Unknown input format\n");
	
		return -1;
	}
		

	i=sortSDChannels();
	if (i<0)
	{
		printf("Error during sortSDChannels()");
		return -1;
	}
	
	i=calculateSDChannelsendtime();
	if (i<0)
	{
		printf("Error during calculateSDChannelsendtime()");
		return -1;
	}

	
	i=retreiveHDData();
	if (i<0)
	{
		printf("Error during retreiveHDData()");
		return -1;
	}

	
	i=generateData();
	if (i<0)
	{
		printf("Error during generateData()");
		return -1;
	}
	
	
	i=checkData();
	if (i<0)
	{
		printf("Error during checkData()");
		return -1;
	}

	
	return 0;
}

int loadChannelsConfigData()
{
	
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseFile(theEPGConfiguration.channel_config_file);
		
	if (doc == NULL ) {
		printf("Document not parsed successfully.\n");
		return -1;
	}
	  
	cur = xmlDocGetRootElement(doc);
		
	if (cur == NULL) {
		printf("Empty Document.\n");
		xmlFreeDoc(doc);
		return -1;
	}
	  
	xmlNode *cur_node = cur;
	  
	while (cur_node != NULL) 
	{
	  
		if (cur_node->type == XML_ELEMENT_NODE) 
		{
			
			if (!xmlStrcmp(cur_node->name, (const xmlChar *) "channels")) 
			{
				xmlNode *channel_node=cur_node->children;
				
				while (channel_node != NULL) 
				{
	
					if (!xmlStrcmp(channel_node->name, (const xmlChar *) "channel")) 
					{
						
						xmlChar *theID= xmlGetProp(channel_node, (const xmlChar *) "id");
						
						channelConfig *thechannelConfig;
							
						thechannelConfig=(channelConfig *)malloc (sizeof(channelConfig) );
						asprintf  (&(thechannelConfig->channel_name), "%s", theID);
	
						thechannelConfigList = g_list_append(thechannelConfigList, thechannelConfig);
						
						xmlFree(theID);
		
					}				
						
					
					channel_node = channel_node->next;
						
				}
			}
		}
		
		cur_node = cur_node->next;
	}
	
	xmlFreeDoc(doc);
	xmlCleanupParser();	
	
	printf("Channel configuration loaded. Number of channels in config file: %d.\n",(int)g_list_length (thechannelConfigList));
	return 0;

}


int generateData()
{
	int i=0;
	int j=0;
	
	FILE* theOutputFile = fopen(theEPGConfiguration.output_unzipped_file, "w"); // should check the result 
	
	if (!theOutputFile) 
	{
		fprintf(stderr, "ERROR: could not open textfile: %s\n", theEPGConfiguration.output_unzipped_file);
		return -1;
	}
  	
	if (theOutputFile!=NULL)
	{
	  
		printf("Generating final data.\n");
	  
		fprintf(theOutputFile,"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
		
		char *time=getTime();
		fprintf(theOutputFile,"<!-- EPG INFO for Movistar + Spain. File automatically generated on %s.  -->\n",time);
		free(time);
		fprintf(theOutputFile,"<!DOCTYPE tv SYSTEM \"xmltv.dtd\">\n");
		fprintf(theOutputFile,"<tv source-info-url=\"http://comunicacion.canalplus.es\" source-info-name=\"Movistar + Spain\" generator-info-name=\"lgarrido\" generator-info-url=\"http://www.xmltv.org/\">\n");
   
		for( i = 0; i < last_SD_channel_index; i = i + 1 )
		{
			
			char *channel_name=normalizeString(channel_list_SD[i].channel_name);
			fprintf(theOutputFile,"	<channel id=\"%s\">\n",channel_name);
			fprintf(theOutputFile,"		<display-name lang=\"es\">%s</display-name>\n",channel_name);
			fprintf(theOutputFile,"	</channel>\n");
			free(channel_name);

		}
		
		fflush(theOutputFile);

		
		for( i = 0; i < last_HD_channel_index; i = i + 1 )
		{
			
			char *channel_name=normalizeString(channel_list_HD[i].channel_name);
			fprintf(theOutputFile,"	<channel id=\"%s\">\n",channel_name);
			fprintf(theOutputFile,"		<display-name lang=\"es\">%s</display-name>\n",channel_name);
			fprintf(theOutputFile,"	</channel>\n");
			free(channel_name);

		}
		
		fflush(theOutputFile);
		
		for( i = 0; i < last_SD_channel_index; i = i + 1 )
		{
			
		
			printf("	Generating final data for channel %s. Number of programs is %d.\n",channel_list_SD[i].channel_name,g_list_length (channel_list_SD[i].programlist));
						
			GList *elem;
			for(elem = channel_list_SD[i].programlist; elem; elem = elem->next) 
			{
				program * theNextProgram;
				theNextProgram= (program*)elem->data;
				char *start_time=getStartTime(theNextProgram->start_date,theNextProgram->start_time );
				char *end_time=getStartTime(theNextProgram->end_date,theNextProgram->end_time );
				char *channel_name=normalizeString(channel_list_SD[i].channel_name);
				fprintf(theOutputFile,"<programme start=\"%s\" stop=\"%s\" channel=\"%s\">\n",start_time,end_time,channel_name);
				free(start_time);
				free(end_time);					
				if (theNextProgram -> title != NULL)
				{
					char *title=normalizeString(theNextProgram -> title);
					fprintf(theOutputFile,"	<title lang=\"es\">%s</title>\n",title);
					free(title);
				}
				else
					fprintf(theOutputFile,"	<title lang=\"es\"></title>\n");
				if (theNextProgram -> description != NULL)
				{
					char *description=normalizeString(theNextProgram -> description);
					fprintf(theOutputFile,"	<desc lang=\"es\">%s.</desc>\n",description);
					free(description);
				}
				else
					fprintf(theOutputFile,"	<desc lang=\"es\"></desc>\n");
				fprintf(theOutputFile,"</programme>\n");
		
			}
			
			fflush(theOutputFile);
		}
	
	
	
		for( i = 0; i < last_HD_channel_index; i = i + 1 )
		{
			j=channel_list_HD[i].SD_channel;
			
			printf("	Generating final data for channel %s. Number of programs is %d.\n",channel_list_HD[i].channel_name,g_list_length (channel_list_SD[j].programlist));
			
			GList *elem;
			for(elem = channel_list_SD[j].programlist; elem; elem = elem->next) 
			{
				program * theNextProgram;
				theNextProgram= (program*)elem->data;
				char *start_time=getStartTime(theNextProgram->start_date,theNextProgram->start_time );
				char *end_time=getStartTime(theNextProgram->end_date,theNextProgram->end_time );
				char *channel_name=normalizeString(channel_list_HD[i].channel_name);
				fprintf(theOutputFile,"<programme start=\"%s\" stop=\"%s\" channel=\"%s\">\n",start_time,end_time,channel_name);
				free(start_time);
				free(end_time);		
				if (theNextProgram -> title != NULL)
				{
					char *title=normalizeString(theNextProgram -> title);
					fprintf(theOutputFile,"	<title lang=\"es\">%s</title>\n",title);
					free(title);
				}
				else
					fprintf(theOutputFile,"	<title lang=\"es\"></title>\n");
				if (theNextProgram -> description != NULL)
				{
					char *description=normalizeString(theNextProgram -> description);
					fprintf(theOutputFile,"	<desc lang=\"es\">%s.</desc>\n",description);
					free(description);
				}
				else
					fprintf(theOutputFile,"	<desc lang=\"es\"></desc>\n");
				fprintf(theOutputFile,"</programme>\n");
	
			}
			
			fflush(theOutputFile);
			
		}
	

		fprintf(theOutputFile,"</tv>\n");


		fclose(theOutputFile);
		
		printf("Finished generation final data.\n");
		
		return 0;
  }
  else 
  {
	  fprintf(stderr, "ERROR: could not open textfile: %s\n", theEPGConfiguration.output_unzipped_file);
	  return -1;
  }
  
}

int retreiveSDDatainTXT()
{

  char cadena[channel_length];
  char input_line[line_length];
  program *theNextPrograma = NULL;
  
  last_SD_channel_index=0;
      
  printf("Starting processing EPG data downloaded from the web.\n");
   
  FILE* file = fopen(theEPGConfiguration.downloaded_file, "r"); 
	
  if (!file) {
         printf("ERROR: could not open textfile: %s\n", theEPGConfiguration.downloaded_file);
         return -1;
  }
	

  int num_programs=0;

  
  if (file)
	{
		while (fgets(input_line, sizeof(input_line), file) != NULL ) 
		{
				if (findSubstring(input_line, "Titulo: ")==0)
				{

					theNextPrograma =(program *) malloc (sizeof(program));
					if (theNextPrograma!=NULL)
					{
							theNextPrograma -> title=NULL;
							theNextPrograma -> description=NULL;
							theNextPrograma -> theTimeStamp=0;
							theNextPrograma -> title = (char *) malloc( strlen(input_line)-strlen("Titulo: ") );
							theNextPrograma -> title [0]='\0';
							strncat(theNextPrograma -> title, &input_line[strlen("Titulo: ")], strlen(input_line)-strlen("Titulo: ")-2);
					}
				}
				else if (findSubstring  (input_line, "Fecha: ")==0)
				{

					if (theNextPrograma!=NULL)
					{
						theNextPrograma -> start_date [0]='\0';
						strncat(theNextPrograma -> start_date, &input_line[strlen("Fecha: ")], strlen(input_line)-strlen("Fecha: ")-2);
					}
				}	
				else if (findSubstring  (input_line, "Hora: ")==0)
				{

					if (theNextPrograma!=NULL)
					{
						theNextPrograma -> start_time [0]='\0';
						strncat(theNextPrograma -> start_time, &input_line[strlen("Hora: ")], strlen(input_line)-strlen("Hora: ")-2);
					}

				}
				else if (findSubstring  (input_line, "Cadena: ")==0)
				{

						cadena [0]='\0';
						strncat(cadena, &input_line[strlen("Cadena: ")], strlen(input_line)-strlen("Cadena: ")-2);
				}
				else if (findSubstring  (input_line, "Descripcion: ")==0)
				{

				}					
				else if (findSubstring  (input_line, "Sinopsis: ")==0)
				{

					if (theNextPrograma!=NULL)
					{
						theNextPrograma -> description = (char *) malloc( strlen(input_line)-strlen("Sinopsis: ") );
						theNextPrograma -> description [0]='\0';
						strncat(theNextPrograma -> description, &input_line[strlen("Sinopsis: ")], strlen(input_line)-strlen("Sinopsis: ")-2);
					}

				}	
				else
				{				
					if (theNextPrograma!=NULL)
					{

							num_programs++;
							generateProgramStartDateandTime(theNextPrograma);	
							addProgramm(cadena,theNextPrograma);	
							//printthePrograma(theNextPrograma);
							theNextPrograma = NULL;

					}		
				}
		}
		
	
		fclose(file);
		
		printf("	Finished calculating End Times\n");
		printf("Finished getting EPG data from the web. Total programs imported: %d\n",num_programs);
		
		return 0;
	}
	else 
	{
        printf("ERROR: could not open textfile: %s\n", theEPGConfiguration.downloaded_file);
		return -1;
	}
	
}

xmlXPathObjectPtr getnodeset (xmlDocPtr doc, xmlChar *xpath){
	
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;

	context = xmlXPathNewContext(doc);
	if (context == NULL) {
		printf("Error in xmlXPathNewContext\n");
		return NULL;
	}
	result = xmlXPathEvalExpression(xpath, context);
	xmlXPathFreeContext(context);
	if (result == NULL) {
		printf("Error in xmlXPathEvalExpression\n");
		return NULL;
	}
	if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
		xmlXPathFreeObject(result);
                printf("No result\n");
		return NULL;
	}
	return result;
}


int retreiveSDDatainXML()
{
	
	program *theNextPrograma = NULL;
  
	int num_programs_current_channel=0;
	int num_programs=0;
	int i;

  
	last_SD_channel_index=0;
  
  
	printf("Starting processing EPG data downloaded from the web.\n");
  
	xmlDocPtr doc;
	xmlNodeSetPtr nodeset;
	xmlNodePtr nodeLevel;

	doc = xmlParseFile(theEPGConfiguration.downloaded_file);
	
	if (doc == NULL ) {
			printf("Document not parsed successfully.\n");
			return -1;
	}
  
  	xmlChar *thecadena;
	xmlChar *thestart_time;
	xmlChar *thefecha;
	xmlChar *thetitle;
	xmlChar *thedescription;
	xmlChar *xpath = (xmlChar*) "//pase";
	
	xmlXPathObjectPtr result = getnodeset (doc, xpath);
	if (result) {
		
		nodeset = result->nodesetval;
		for (i=0; i < nodeset->nodeNr; i++) 
		{
			thecadena = xmlGetProp(nodeset->nodeTab[i], (const xmlChar *) "cadena");
			thefecha = xmlGetProp(nodeset->nodeTab[i], (const xmlChar *) "fecha");

			
			for(	nodeLevel = nodeset->nodeTab[i]->children; nodeLevel != NULL; nodeLevel = nodeLevel->next)
			{
				if (!xmlStrcmp(nodeLevel->name, (const xmlChar *) "hora"))
				{	
					thestart_time = xmlNodeListGetString(doc, nodeLevel->xmlChildrenNode, 1);
				}
				if (!xmlStrcmp(nodeLevel->name, (const xmlChar *) "descripcion_corta"))
				{	
					thetitle = xmlNodeListGetString(doc, nodeLevel->xmlChildrenNode, 1);
				}
				if (!xmlStrcmp(nodeLevel->name, (const xmlChar *) "sinopsis_corta"))
				{	
					thedescription = xmlNodeListGetString(doc, nodeLevel->xmlChildrenNode, 1);
				}
								
			}
			
			num_programs++;
						
			theNextPrograma =(program *) malloc (sizeof(program));
			generateProgramData((char *)thefecha,(char *)thestart_time,(char *)thetitle,(char *)thedescription,theNextPrograma);
				
			addProgramm((char*)thecadena,theNextPrograma);	
			
			xmlFree(thecadena);
			xmlFree(thefecha);							
			xmlFree(thestart_time);
			xmlFree(thetitle);
			xmlFree(thedescription);

		}
		xmlXPathFreeObject (result);
		
	}
	else
	{
		printf("Document not parsed successfully.\n");
		return -1;
		
	}

	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	printf("	Finished getting EPG data from %s. Number of programs imported: %d\n", channel_list_SD[last_SD_channel_index-1].channel_name,num_programs_current_channel);


	if (num_programs==0)
	{
		
		printf("ERROR: num_programs=0. Something went wrong.\n");
		return -1;
	}	
	else
		return 0;

	printf("Finished getting EPG data from the web. Total programs imported: %d\n",num_programs);
	
	return 0;
	
}
void addProgramm(char * thecadena,program * theNextProgram)
{
	

	char * theCadenaNormalizada=normalizeChannelName((char *)thecadena);
	int i=returnSDchannel((char *)theCadenaNormalizada);
	if(i < 0 )
	{
			strcpy (channel_list_SD[last_SD_channel_index].channel_name,(char *)theCadenaNormalizada);
			channel_list_SD[last_SD_channel_index].programlist = g_list_append(channel_list_SD[last_SD_channel_index].programlist, theNextProgram);
			//printf("NEW CHANNEL FOUND: *%s*. \n", channel_list_SD[last_SD_channel_index].channel_name);

			
			last_SD_channel_index++;
	}
	else
	{
			//printf("EXISTING CHANNEL: *%s*. NEW PROGRAMM: *%s*\n", channel_list_SD[i].channel_name,theNextProgram -> title);
			channel_list_SD[i].programlist = g_list_append(channel_list_SD[i].programlist, theNextProgram);
			
	}	

	free(theCadenaNormalizada);	
}
						
int sortSDChannels()
{
	int i;
	printf("	Sorting channels\n");
	for( i = 0; i < last_SD_channel_index; i = i + 1 )
	{
		channel_list_SD[i].programlist = g_list_sort(channel_list_SD[i].programlist, (GCompareFunc)programsComparator);
	}

	return 0;
}

int calculateSDChannelsendtime()
{
	int i;
	printf("	Calculating End Times\n");
	for( i = 0; i < last_SD_channel_index; i = i + 1 )
	{
		calculateEndTime(&channel_list_SD[i]);
	}
	printf("	Finished calculating End Times\n");


	return 0;
}

int retreiveHDData()
{
	

	char s[2] = ",";
	char *token;
	int i=-1;
   
	last_HD_channel_index=0;
	token = strtok(theEPGConfiguration.copy_to_hd, s);
   
	printf("Copying EPG programs from SD channels to HD channels\n");
	while( token != NULL ) 
	{
		i=returnSDchannel(token);
		if (i<0)
		{
			printf("SD channel *%s* cannot be copied to an HD channel. No data for the SD channel.\n", token);
		}
		else
		{
			memcpy( channel_list_HD[last_HD_channel_index].channel_name, token, strlen(token));
			strncat (channel_list_HD[last_HD_channel_index].channel_name, "HD", 2 );
			channel_list_HD[last_HD_channel_index].SD_channel=i;
			
			printf("	Data copied. Source=%s. Destination=%s\n",token,channel_list_HD[last_HD_channel_index].channel_name);
			last_HD_channel_index++;
		}
	
	
		token = strtok(NULL, s);
	}
	printf("Finished copying EPG programs from SD channels to HD channels\n");
	return 0;
}

int returnSDchannel (char channel[])
{
	int enc=0;
	int i=0;
	while( (i < last_SD_channel_index) && (enc==0 ))
	{
		if ( strcmp ( channel, channel_list_SD[i].channel_name ) ==0)
		{
				enc=1;
		}
		else
		{
			i++;
		}
	}
	if (enc>0)
	{
		return i;
	}
	else
	{
		return -1;
	}
}

int returnHDchannel (char channel[])
{
	int enc=0;
	int i=0;
	while( (i < last_HD_channel_index) && (enc==0 ))
	{
		if ( strcmp ( channel, channel_list_HD[i].channel_name ) ==0)
		{
				enc=1;
		}
		else
		{
			i++;
		}
	}
	if (enc>0)
	{
		return i;
	}
	else
	{
		return -1;
	}
}


gint channelConfig_comparator_1 (gpointer a, gpointer b)
{
	return strcmp( (char *)((channelConfig *)a)->channel_name, (char *)((channelConfig *)b)->channel_name );

}

int checkData()
{

		int i;			
		
		GList *elem;
			
		for(elem = thechannelConfigList; elem; elem = elem->next) 
		{
			
			channelConfig *thechannelConfig;
			thechannelConfig= (channelConfig*) elem->data;
			
			if ( (returnSDchannel(thechannelConfig ->channel_name)<0) && (returnHDchannel(thechannelConfig ->channel_name)<0) )
			{
					printf("WARNING: No EPG data found for %s. EPG data will be empty for the channel.\n",thechannelConfig ->channel_name);
			}
			
		}
	
	
		
		for( i = 0; i < last_SD_channel_index; i = i + 1 )
		{
			if (isChannelInchannelConfig(channel_list_SD[i].channel_name,thechannelConfigList)<=0)
			{
					printf("WARNING: No channel configuration found for %s. EPG data will be useless.\n",channel_list_SD[i].channel_name);
			}
		}
		
		for( i = 0; i < last_HD_channel_index; i = i + 1 )
		{
			if (isChannelInchannelConfig(channel_list_HD[i].channel_name,thechannelConfigList)<=0)
			{
					printf("WARNING: No channel configuration found for %s. EPG data will be useless.\n",channel_list_HD[i].channel_name);
			}
		}
		
		return 0;
}
