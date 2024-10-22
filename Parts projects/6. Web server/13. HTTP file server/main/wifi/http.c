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


static const char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest) {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash) {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize) {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest + base_pathlen;
}
// --------------------------------------------------------------------------------------------------------------------
static esp_err_t get_file_list_handler(httpd_req_t *req)
{
	const static char *TAG = "GET FILE LIST HANDLER";
	
    const char* dirpath = "/data";
    struct dirent *entry;
    struct stat entry_stat;
    char entrypath[300];
    char entrysize[16];

    DIR *dir = opendir(dirpath);
    if (dir == NULL)
    {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_sendstr_chunk(req, "<table border=\"1\"><tr><th>Name</th><th>Type</th><th>Size</th></tr>");
    
    while ((entry = readdir(dir)) != NULL)			 // Повторювати з кожним файлом
    {
		// Визначити тип файлу
		const char *entrytype = (entry->d_type == DT_DIR) ? "Directory" : "File";
        sprintf(entrysize, "%ld", entry_stat.st_size);
		
		// Згенерувати шлях до файлу
        snprintf(entrypath, sizeof(entrypath), "%s/%s", dirpath, entry->d_name);
        
      	// Отримати інформацію про файл
        if (stat(entrypath, &entry_stat) == -1) 
        {
        	ESP_LOGE(TAG, "Failed to stat %s : %s", entrytype, entry->d_name);
        	continue;
    	}
        
        snprintf(entrysize, sizeof(entrysize), "%ld", entry_stat.st_size);

	    // Створюємо рядок для таблиці
	    httpd_resp_sendstr_chunk(req, "<tr>");
	    
	    // Виводимо ім'я файлу
	    httpd_resp_sendstr_chunk(req, "<td>");
	    httpd_resp_sendstr_chunk(req, entry->d_name);
	    httpd_resp_sendstr_chunk(req, "</td>");
	    
	    // Виводимо тип файлу (директорія чи файл)
	    httpd_resp_sendstr_chunk(req, "<td>");
	    httpd_resp_sendstr_chunk(req, entrytype);
	    httpd_resp_sendstr_chunk(req, "</td>");
	    
	    // Виводимо розмір файлу
	    httpd_resp_sendstr_chunk(req, "<td>");
	    httpd_resp_sendstr_chunk(req, entrysize);
	    httpd_resp_sendstr_chunk(req, "</td>");
	    
	    // Додаємо кнопку "Delete"
	    httpd_resp_sendstr_chunk(req, "<td><form method=\"get\" action=\"/delete/");
	    httpd_resp_sendstr_chunk(req, entry->d_name);
	    httpd_resp_sendstr_chunk(req, "\"><button type=\"submit\">Delete</button></form></td>");
	    
	    // Додаємо кнопку "Download"
	    httpd_resp_sendstr_chunk(req, "<td><form method=\"get\" action=\"/download/");
	    httpd_resp_sendstr_chunk(req, entry->d_name);
	    httpd_resp_sendstr_chunk(req, "\"><button type=\"submit\">Download</button></form></td>");
	    
	    // Закриваємо рядок таблиці
	    httpd_resp_sendstr_chunk(req, "</tr>\n");
    }
    closedir(dir);
  
    httpd_resp_sendstr_chunk(req, "</tbody></table>");
    httpd_resp_send_chunk(req, NULL, 0);  					// Завершуємо відповідь
    return ESP_OK;
}
// --------------------------------------------------------------------------------------------------------------------
static esp_err_t delete_file_handler(httpd_req_t *req)
{
	ESP_LOGI("DELETE_HANDLER", "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<_---------------");
	
   	// char filepath[FILE_PATH_MAX];
   	char filepath[FILE_PATH_MAX];
   	const char* base_path = "/data";  // Базовий шлях до директорії
   	const char* uri = req->uri;       // URI, який приходить із запиту
    
    // Отримати коректний шлях до файлу без зайвих символів та без частини '/delete/'
    const char *filename = get_path_from_uri(filepath, base_path, uri + sizeof("/delete") - 1, sizeof(filepath));
    if (!filename) 
    {
        ESP_LOGE("DELETE_HANDLER", "Failed to get file path");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    ESP_LOGI("DELETE_HANDLER", "Deleting filename: %s <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<", filename);
    ESP_LOGI("DELETE_HANDLER", "Deleting file path: %s <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<", filepath);
    
    struct stat file_stat;
    if (stat(filepath, &file_stat) == -1)
    {
        ESP_LOGE("DELETE_HANDLER", "File does not exist: %s", filepath);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // Delete file
    if (unlink(filepath) == 0)
    {
        ESP_LOGI("DELETE_HANDLER", "File deleted successfully: %s", filepath);
        
        // Виконуємо перенаправлення на головну сторінку після видалення
        httpd_resp_set_status(req, "303 See Other");
        httpd_resp_set_hdr(req, "Location", "/");
        httpd_resp_sendstr(req, "File deleted successfully");
    } 
    else 
    {
        ESP_LOGE("DELETE_HANDLER", "Failed to delete file: %s", filepath);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    return ESP_OK;
}
// --------------------------------------------------------------------------------------------------------------------
static esp_err_t download_file_handler(httpd_req_t *req)
{
	ESP_LOGI("DOWNLOAD_HANDLER", "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<_---------------");
	
	char filepath[FILE_PATH_MAX];
	const char* base_path = "/data";  // Базовий шлях до директорії
	const char* uri = req->uri;       // URI, який приходить із запиту
	
	// Отримати коректний шлях до файлу без зайвих символів та без частини '/delete/'
    const char *filename = get_path_from_uri(filepath, base_path, uri + sizeof("/download") - 1, sizeof(filepath));
    if (!filename) 
    {
        ESP_LOGE("DOWNLOAD_HANDLER", "Failed to get file path");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
	
	ESP_LOGI("DOWNLOAD_HANDLER", "Download filename: %s <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<", filename);
    ESP_LOGI("DOWNLOAD_HANDLER", "Download file path: %s <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<", filepath);
	
	FILE *file = fopen(filepath, "r");
	if(!file)
	{
		httpd_resp_send_404(req);
		return ESP_FAIL;		
	}	
	
	// Встановити тип відповіді залежго від формату файлу
	httpd_resp_set_type(req, "application/octet-stream");
	// Втановити заголовок, щоб браузер запропонував завантаження файлу
	char header[100] = {0,};
	snprintf(header, sizeof(header),  "attachment; filename=\"%s\"", filename);
	httpd_resp_set_hdr(req, "Content-Disposition", header);
	
	char buffer[1024];
	size_t chunksize;
	
	// читати і передавати файл частинами
	while((chunksize = fread(buffer, 1, sizeof(buffer), file)) > 0)
	{
		if(httpd_resp_send_chunk(req, buffer, chunksize) != ESP_OK)
		{
			fclose(file);
			httpd_resp_send_500(req);
			return ESP_FAIL;
		}
	}
	
	fclose(file);
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}
// --------------------------------------------------------------------------------------------------------------------
// Завантаження HTML сторінки
static esp_err_t index_html_handler(httpd_req_t *req) 
{

    ///////////////////////////////////////////////////////////////////////////////////////////
    
     const static char *TAG_HTML_HANDLER = "HTML HANDLER";
	
    FILE* f = fopen("/spiffs/index.html", "r");
    
    if (f == NULL)
    {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    else
    {
		ESP_LOGI(TAG_HTML_HANDLER, "OPEN FILE index.html");
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
	
	
	// Files
	 httpd_uri_t file_list_uri = {
    	.uri = "/get_file_list",
    	.method = HTTP_GET,
    	.handler = get_file_list_handler
	};
	httpd_register_uri_handler(server, &file_list_uri);
	
	httpd_uri_t delete_file_uri = {
    	.uri = "/delete/*",  // Маршрут для видалення файлів
    	.method = HTTP_GET,
    	.handler = delete_file_handler
	};
	httpd_register_uri_handler(server, &delete_file_uri);
	
	httpd_uri_t download_file_uri = {
		.uri = "/download/*",
		.method = HTTP_GET,
		.handler = download_file_handler
	};
	httpd_register_uri_handler(server, &download_file_uri);


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





