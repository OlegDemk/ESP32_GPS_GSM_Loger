/*
 * udp.c
 *
 *  Created on: May 18, 2023
 *      Author: odemki
 */

#include "http.h"


static const char *TAG = "http";

extern QueueHandle_t bme280_queue;
extern QueueHandle_t GPS_queue;
extern QueueHandle_t battery_queue;


// --------------------------------------------------------------------------------------------------------------------
// Обробник для передачі стану світлодіодів у форматі JSON
static esp_err_t get_bme280_data_handler(httpd_req_t *req)
{
	const static char *TAG_SEND_DATA = "SEND BME280 DATA JSON";
	
    char response[200];
    
    bme280_thp_t bme280_thp_queue;
    
   	bool data_received = false;
   	
   	data_received = xQueueReceive(bme280_queue, &bme280_thp_queue, pdMS_TO_TICKS(100));			// Get new BME280 data
   
    snprintf(response, sizeof(response),
             "{\"temperature\":%0.1f, \"humidity\":%0.f, \"preassure\":%0.f}",
             data_received ? bme280_thp_queue.temperature : 0.0, 
             data_received ? bme280_thp_queue.humidity : 0.0, 
             data_received ? bme280_thp_queue.preassure : 0.0);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
      
    return ESP_OK;
}
// --------------------------------------------------------------------------------------------------------------------
static esp_err_t get_gps_data_handler(httpd_req_t *req)
{
	const static char *TAG_SEND_DATA = "SEND GPS DATA JSON";
	
    char response[300];
    
    gps_data_gps_t gps_data_gps;
    
   	bool data_received = false;
   	
   	data_received = xQueueReceive(GPS_queue, &gps_data_gps, pdMS_TO_TICKS(100));			// Get new BME280 data
   
   	ESP_LOGI(TAG_SEND_DATA, "Send data: La:%.5f, Lo:%.5f, altitude:%.5f",
   	 gps_data_gps.latitude, gps_data_gps.longitude, gps_data_gps.altitude);
   
   	// Тут передавати дані
    snprintf(response, sizeof(response),
       	 "{\n"
       	 " \"latitude\": %0.5f, \"longitude\": %0.5f, \"altitude\": %0.1f,\n"
       	 " \"speed\": %0.2f,\n"
       	 " \"sats_in_view\": %f,\n"
       	 " \"hour\": %d,\n"
       	 " \"minute\": %d,\n"
       	 " \"second\": %d,\n"
       	 " \"day\": %d,\n"
       	 " \"month\": %d,\n"
       	 " \"year\": %d\n"
       	 "}",       
             data_received ? gps_data_gps.latitude : 0.0, 
             data_received ? gps_data_gps.longitude : 0.0,
             data_received ? gps_data_gps.altitude : 0.0,
             data_received ? gps_data_gps.speed : 0.0,
             data_received ? gps_data_gps.sats_in_view : 0.0,
             data_received ? gps_data_gps.time.hour : 0,
             data_received ? gps_data_gps.time.minute : 0,
             data_received ? gps_data_gps.time.second : 0,
             data_received ? gps_data_gps.date.day : 0,
             data_received ? gps_data_gps.date.month : 0,
             data_received ? gps_data_gps.date.year : 0);
             
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
     
    return ESP_OK;
}
// --------------------------------------------------------------------------------------------------------------------
static bool gps_enabled = false;

static esp_err_t toggle_gps_handler(httpd_req_t *req)
{
	const static char *TAG_GPS_BUTTON_DATA = "GPS BUTTON";
	ESP_LOGI(TAG_GPS_BUTTON_DATA, "BUTTON !!!!!!!!!!!!!! ");
		
	char buffer[100];
	int ret = httpd_req_recv(req, buffer, sizeof(buffer));
	
	if(ret <= 0)
	{
		httpd_resp_send_500(req);
		return ESP_FAIL;
	}
	cJSON *json = cJSON_Parse(buffer);
	if(json == NULL)
	{
		httpd_resp_set_status(req, "400 Bad Request");
		httpd_resp_send(req, NULL, 0);
		return ESP_FAIL;
	}
	
	cJSON *status = cJSON_GetObjectItem(json, "status");
	if(cJSON_IsBool(status))
	{
		gps_enabled = cJSON_IsTrue(status);
		
		if(gps_enabled)
		{
			ESP_LOGI(TAG_GPS_BUTTON_DATA,"GPS ENABLED  <<<<<<<<<<<<<<<<<<<<<<<");
		}
		else
		{
			ESP_LOGI(TAG_GPS_BUTTON_DATA,"GPS DISABLED  <<<<<<<<<<<<<<<<<<<<<<<");
		}		
	}
	
	cJSON *response = cJSON_CreateObject();
	cJSON_AddBoolToObject(response, "gps_status", gps_enabled);
	
	const char *response_str = cJSON_Print(response);
	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, response_str, HTTPD_RESP_USE_STRLEN);
	
	cJSON_Delete(response);
	cJSON_Delete(json);
	
	
	return ESP_OK;
}

// --------------------------------------------------------------------------------------------------------------------
static esp_err_t get_battery_data_handler(httpd_req_t *req)
{
	const static char *TAG_SEND_DATA = "SEND BATTERY DATA JSON";
    char response[100];
    battery_t batterty;
   	bool data_received = false;
   	
   	data_received = xQueueReceive(battery_queue, &batterty, pdMS_TO_TICKS(100));			// Get new BME280 data
   
    snprintf(response, sizeof(response),
             "{\"battery_level\":%f}",
             data_received ? batterty.battery_level : 0.0);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
      
    return ESP_OK;
}
// --------------------------------------------------------------------------------------------------------------------
// Завантаження HTML сторінки
static esp_err_t index_html_handler(httpd_req_t *req) 
{
    FILE* f = fopen("/spiffs/index.html", "r");
    
    if (f == NULL)
    {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char chunk[128];
    size_t chunksize;
    
    httpd_resp_set_type(req, "text/html");

    do{
        chunksize = fread(chunk, 1, sizeof(chunk), f);
        if (chunksize > 0) 
        {
            httpd_resp_send_chunk(req, chunk, chunksize);
        }
    } while (chunksize != 0);

    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0); // Закінчити відповідь
    return ESP_OK;
}

// ---------------------------------------------------------------------------------------------------------------------
httpd_handle_t start_webserver(void)
{
	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.lru_purge_enable = true;
	
	  static struct file_server_data *server_data = NULL;
	  if(server_data)
	  {
		  ESP_LOGE(TAG, "File server already started");
		  return ESP_ERR_INVALID_STATE;
	  }
	
	  server_data = calloc(1, sizeof(struct file_server_data));
	  if(!server_data)
	  {
		  ESP_LOGE(TAG, "Failed to allocate memory for server data");
		  return ESP_ERR_NO_MEM;
	  }
	  strlcpy(server_data->base_path, "/spiffs", sizeof(server_data->base_path));
	  config.uri_match_fn = httpd_uri_match_wildcard;
	  config.lru_purge_enable = true;
	
	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	
	  if(httpd_start(&server, &config) != ESP_OK)
	  {
		  ESP_LOGE(TAG, "Error starting server!");
		  return NULL;
	  }
	
	ESP_LOGI(TAG, "Register URL handlers");

	httpd_uri_t index_html_uri = {
	    .uri = "/",
	    .method = HTTP_GET,
	    .handler = index_html_handler
	};

	httpd_uri_t status_get_bme280_data_uri = {
	    .uri = "/get_sensor_data",
	    .method = HTTP_GET,
	    .handler = get_bme280_data_handler
	};
	
	httpd_uri_t status_get_gps_data_uri = {
	    .uri = "/get_gps_data",
	    .method = HTTP_GET,
	    .handler = get_gps_data_handler
	};
	
	httpd_uri_t status_get_battery_data_uri = {
	    .uri = "/get_battery_level_data",
	    .method = HTTP_GET,
	    .handler = get_battery_data_handler
	};
	
	httpd_uri_t toggle_gps_uri = {
		.uri = "/toggle_gps",
		.method = HTTP_POST,
		.handler = toggle_gps_handler,
		.user_ctx = NULL
	};


	httpd_register_uri_handler(server, &index_html_uri);
	httpd_register_uri_handler(server, &status_get_bme280_data_uri);
	httpd_register_uri_handler(server, &status_get_gps_data_uri);
	httpd_register_uri_handler(server, &status_get_battery_data_uri);
  	httpd_register_uri_handler(server, &toggle_gps_uri);
  
 	return server;
}
// ---------------------------------------------------------------------------------------------------------------------
void stop_webserver(httpd_handle_t server)
{
    httpd_stop(server);
}
// ---------------------------------------------------------------------------------------------------------------------





