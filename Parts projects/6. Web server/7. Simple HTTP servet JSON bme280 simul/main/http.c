/*
 * udp.c
 *
 *  Created on: May 18, 2023
 *      Author: odemki
 */

#include "http.h"


static const char *TAG = "http";

extern QueueHandle_t bme280_queue;



// --------------------------------------------------------------------------------------------------------------------
// Обробник для передачі стану світлодіодів у форматі JSON
static esp_err_t get_bme280_data_handler(httpd_req_t *req)
{
	const static char *TAG_SEND_DATA = "SEND DATA JSON";
	
    char response[200];
    
    bme280_thp bme280_thp_queue_t;
    
   	bool data_received = false;
   	
   	data_received = xQueueReceive(bme280_queue, &bme280_thp_queue_t, pdMS_TO_TICKS(100));
   	
   	//ESP_LOGI(TAG_SEND_DATA, "Send data: T:%.1f, H:%.1f, P:%.1f", bme280_thp_queue_t.temperature, bme280_thp_queue_t.humidity, bme280_thp_queue_t.preasure);
    
    snprintf(response, sizeof(response),
             "{\"temperature\":%0.1f, \"humidity\":%0.f, \"preassure\":%0.f}",
             data_received ? bme280_thp_queue_t.temperature : 0.0, 
             data_received ? bme280_thp_queue_t.humidity : 0.0, 
             data_received ? bme280_thp_queue_t.preassure : 0.0);
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

	httpd_uri_t status_get_bme280_data_uri = {
	    .uri = "/get_sensor_data",
	    .method = HTTP_GET,
	    .handler = get_bme280_data_handler
	};

	httpd_uri_t index_html_uri = {
	    .uri = "/",
	    .method = HTTP_GET,
	    .handler = index_html_handler
	};

	httpd_register_uri_handler(server, &status_get_bme280_data_uri);
	httpd_register_uri_handler(server, &index_html_uri);
  
  return server;
}
// ---------------------------------------------------------------------------------------------------------------------
void stop_webserver(httpd_handle_t server)
{
    httpd_stop(server);
}
// ---------------------------------------------------------------------------------------------------------------------





