/*
 * udp.c
 *
 *  Created on: May 18, 2023
 *      Author: odemki
 */

#include "http.h"


static const char *TAG = "http";

// Структура для зберігання стану світлодіодів
typedef struct {
    bool red;
    bool green;
    bool blue;
} led_status_t;

static led_status_t led_status = { false, false, false };

// --------------------------------------------------------------------------------------------------------------------
// Обробник для передачі стану світлодіодів у форматі JSON
static esp_err_t get_status_handler(httpd_req_t *req)
{
    char response[100];
    snprintf(response, sizeof(response),
             "{\"red\":%d, \"green\":%d, \"blue\":%d}",
             led_status.red, led_status.green, led_status.blue);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}
// --------------------------------------------------------------------------------------------------------------------
// Обробник для управління світлодіодами через параметри
static esp_err_t control_handler(httpd_req_t *req)
{
    char buf[100];
    char color[10], state[5];

    if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK) 
    {
        httpd_query_key_value(buf, "color", color, sizeof(color));
        httpd_query_key_value(buf, "state", state, sizeof(state));

        int state_val = atoi(state);

        if (strcmp(color, "red") == 0) 
        {
            gpio_set_level(CONFIG_RED_GPIO, state_val);
            led_status.red = state_val;
        } 
        else if (strcmp(color, "green") == 0) 
        {
            gpio_set_level(CONFIG_GREEN_GPIO, state_val);
            led_status.green = state_val;
        } 
        else if (strcmp(color, "blue") == 0) 
        {
            gpio_set_level(CONFIG_BLUE_GPIO, state_val);
            led_status.blue = state_val;
        }
    }
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
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


	httpd_uri_t status_get_uri = {
	    .uri = "/get_status",
	    .method = HTTP_GET,
	    .handler = get_status_handler
	};
	
	httpd_uri_t control_get_uri = {
	    .uri = "/control",
	    .method = HTTP_GET,
	    .handler = control_handler
	};
	
	httpd_uri_t index_html_uri = {
	    .uri = "/",
	    .method = HTTP_GET,
	    .handler = index_html_handler
	};

	httpd_register_uri_handler(server, &status_get_uri);
	httpd_register_uri_handler(server, &control_get_uri);
	httpd_register_uri_handler(server, &index_html_uri);
  
  return server;
}
// ---------------------------------------------------------------------------------------------------------------------
void stop_webserver(httpd_handle_t server)
{
    httpd_stop(server);
}
// ---------------------------------------------------------------------------------------------------------------------





