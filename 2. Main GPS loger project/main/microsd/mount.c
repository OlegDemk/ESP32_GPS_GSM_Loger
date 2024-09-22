/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* HTTP File Server Example, SD card / SPIFFS mount functions.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include "mount.h"

static const char *TAG = "example_mount";

#define LOG_INTO_COMPORT OFF

#ifdef CONFIG_EXAMPLE_MOUNT_SD_CARD
// ----------------------------------------------------------------------------------------
uint8_t log_data(char* base_path, char* file_nmae, char* data, char* log_info)
{
	static const char *SPI_FLASH = "SPI FLASH WRITE DATA";

	char mount_point_buf[30]= {0,};
	memset(mount_point_buf, 0, sizeof(mount_point_buf));
	strcat(mount_point_buf, base_path);

	if(LOG_INTO_COMPORT == ON)
	{
		ESP_LOGI(SPI_FLASH, "mount_point_buf:> %s", mount_point_buf);
	}

	strcat(mount_point_buf, "/");
	strcat(mount_point_buf, file_nmae);

	// ESP_LOGI(SPI_FLASH, "all mount_point_buf:> %s", mount_point_buf);

	FILE *f = fopen(mount_point_buf, "a");							// Відкрити або створити новий файл
	if (f == NULL)
	{
		ESP_LOGE(SPI_FLASH, "File can't be write: %s", log_info);
		//return;
		FILE *f = fopen(mount_point_buf, "w");							// Відкрити або створити новий файл
		if (f == NULL)
		{
			ESP_LOGE(SPI_FLASH, "File can't create log file: %s", log_info);
			return 0;
		}
		else
		{
			if(LOG_INTO_COMPORT == ON)
			{
				ESP_LOGI(SPI_FLASH, "New file was created !");
			}
		}
	}
	else
	{
		if(LOG_INTO_COMPORT == ON)
		{
			ESP_LOGI(SPI_FLASH, "Write %s data", log_info);
		}
		fprintf(f, "%s \n", data);
	}
	fclose(f);
	return 1;
}
// ----------------------------------------------------------------------------------------
esp_err_t example_mount_storage(const char* base_path)
{
    ESP_LOGI(TAG, "Initializing SD card");

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_SDCARD_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // CONFIG_EXAMPLE_FORMAT_IF_MOUNT_SDCARD_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    esp_err_t ret;
    sdmmc_card_t* card;

#ifdef CONFIG_EXAMPLE_USE_SDMMC_HOST
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // To use 1-line SD mode, change this to 1:
    slot_config.width = 4;

#ifdef SOC_SDMMC_USE_GPIO_MATRIX
    // For chips which support GPIO Matrix for SDMMC peripheral, specify the pins.
    slot_config.clk = CONFIG_EXAMPLE_PIN_CLK;
    slot_config.cmd = CONFIG_EXAMPLE_PIN_CMD;
    slot_config.d0 = CONFIG_EXAMPLE_PIN_D0;
    slot_config.d1 = CONFIG_EXAMPLE_PIN_D1;
    slot_config.d2 = CONFIG_EXAMPLE_PIN_D2;
    slot_config.d3 = CONFIG_EXAMPLE_PIN_D3;
#endif // SOC_SDMMC_USE_GPIO_MATRIX

    // Enable internal pullups on enabled pins. The internal pullups
    // are insufficient however, please make sure 10k external pullups are
    // connected on the bus. This is for debug / example purpose only.
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ret = esp_vfs_fat_sdmmc_mount(base_path, &host, &slot_config, &mount_config, &card);

#else // CONFIG_EXAMPLE_USE_SDMMC_HOST

    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = CONFIG_EXAMPLE_PIN_MOSI,
        .miso_io_num = CONFIG_EXAMPLE_PIN_MISO,
        .sclk_io_num = CONFIG_EXAMPLE_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ret;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = CONFIG_EXAMPLE_PIN_CS;
    slot_config.host_id = host.slot;
    ret = esp_vfs_fat_sdspi_mount(base_path, &host, &slot_config, &mount_config, &card);

#endif // !CONFIG_EXAMPLE_USE_SDMMC_HOST

    if (ret != ESP_OK){
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    }

    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}

#else // CONFIG_EXAMPLE_MOUNT_SD_CARD

/* Function to initialize SPIFFS */
esp_err_t example_mount_storage(const char* base_path)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = base_path,
        .partition_label = NULL,
        .max_files = 5,   // This sets the maximum number of files that can be open at the same time
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    return ESP_OK;
}
// ------------------------------------------------------------------------------------------
uint8_t log_data(char* base_path, char* file_nmae, char* data, char* log_info)
{
	static const char *SPI_FLASH = "SPI FLASH WRITE DATA";

	char mount_point_buf[30]= {0,};
	memset(mount_point_buf, 0, sizeof(mount_point_buf));
	strcat(mount_point_buf, base_path);

	if(LOG_INTO_COMPORT == ON)
	{
		ESP_LOGI(SPI_FLASH, "mount_point_buf:> %s", mount_point_buf);
	}

	strcat(mount_point_buf, "/");
	strcat(mount_point_buf, file_nmae);

	// ESP_LOGI(SPI_FLASH, "all mount_point_buf:> %s", mount_point_buf);

	FILE *f = fopen(mount_point_buf, "a");							// Відкрити або створити новий файл
	if (f == NULL)
	{
		ESP_LOGE(SPI_FLASH, "File can't be write: %s", log_info);
		//return;
		FILE *f = fopen(mount_point_buf, "w");							// Відкрити або створити новий файл
		if (f == NULL)
		{
			ESP_LOGE(SPI_FLASH, "File can't create log file: %s", log_info);
			return 0;
		}
		else
		{
			if(LOG_INTO_COMPORT == ON)
			{
				ESP_LOGI(SPI_FLASH, "New file was created !");
			}
		}
	}
	else
	{
		if(LOG_INTO_COMPORT == ON)
		{
			ESP_LOGI(SPI_FLASH, "Write %s data", log_info);
		}
		fprintf(f, "%s", data);
	}
	fclose(f);
	return 1;
}




#endif // !CONFIG_EXAMPLE_MOUNT_SD_CARD

// ------------------------------------------------------------------------------------------
// Loging dsts first time
uint8_t log_data1(char* base_path, char* file_nmae, float latitude, float longitude)
{
	static const char *MEM = "SPI FLASH WRITE DATA";

	//ESP_LOGI(MEM, ">>>>>>>>>>FULL file name: %s ", file_nmae);

	char main_buff[500] = {
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
		"<Document>\n"
		"<name>simple route</name>\n"
		"<Placemark>\n"
		"<name>Start point</name>\n"
		"<Point>\n"
		"<coordinates>"
	};

	char buf_data[100] = {0,};

	sprintf(buf_data, "%.5f,%.5f </coordinates>\n"
		"</Point>\n"
		"</Placemark>\n",
		longitude, latitude);

	strcat(main_buff, buf_data);

	char end_buff[200] = {
		"<Placemark>\n"
		"<name>Маршрут</name>\n"
		"<LineString>\n"
		"<coordinates>\n"
		"</coordinates>\n"
		"</LineString>\n"
		"</Placemark>\n"
		"</Document>\n"
		"</kml>\n"
	};

	strcat(main_buff, end_buff);

	//ESP_LOGI(MEM, "END 3: %s", main_buff);

	log_data("/data", file_nmae, main_buff, "writing data");

	return 0;
}

// ------------------------------------------------------------------------------------------
// Loging tata in the end of file
uint8_t log_data2(char* base_path, char* file_nmae, float latitude, float longitude, int gps_point_counter)
{
	static const char *MEM = "SPI FLASH WRITE DATA";

	char mount_point_buf[30]= {0,};
	memset(mount_point_buf, 0, sizeof(mount_point_buf));
	strcat(mount_point_buf, base_path);

	if(LOG_INTO_COMPORT == ON)
	{
		ESP_LOGI(MEM, "mount_point_buf:> %s", mount_point_buf);
	}

	strcat(mount_point_buf, "/");
	strcat(mount_point_buf, file_nmae);

	// ESP_LOGI(SPI_FLASH, "all mount_point_buf:> %s", mount_point_buf);

	FILE *f = fopen(mount_point_buf, "r+");							// Відкрити або створити новий файл
	if (f == NULL)
	{
		ESP_LOGE(MEM, "File can't be write !");
		//return; ERROR
	}
	else
	{
		uint8_t roe_line_counter = 0;
		uint8_t target_row = 11; 		 // Delete last 11 rows of file
		char main_buff[500] = {0,};

		// Go to the end of file
		if(fseek(f, 0, SEEK_END) != 0)
		{
			ESP_LOGE(MEM, "ERROR on function fseek");
		}

		while(ftell(f) > 0)					// Step up from end of file step by step
		{
			fseek(f, -1, SEEK_CUR);			// Step up on one char
			char ch = fgetc(f);
			if(ch == '\n')					// If was find end of row character
			{
				roe_line_counter++;
				if(roe_line_counter == target_row)
				{
					break;
				}
			}
			fseek(f, -1, SEEK_CUR);			// Step up on one char
		}

		long pos = ftell(f);				// Save pointer on position on file

		// Numeration of points
		char buff_point[200] = {0,};
		sprintf(buff_point ,
			"<Placemark>\n"
			"<name>Point %d"
			"</name>\n"
			"<Point>\n"
			"<coordinates>",
		gps_point_counter);

		strcat(main_buff, buff_point);

		char buf_lantitude[80] = {0,};
		sprintf(buf_lantitude, "%.5f,%.5f </coordinates>\n"
			"</Point>\n"
			"</Placemark>\n",
			longitude, latitude
		);
		strcat(main_buff, buf_lantitude);

		char buff_3[150] = {
			"<Placemark>\n"
			"<name>Маршрут</name>\n"
			"<LineString>\n"
			"<coordinates>\n"
			"</coordinates>\n"
			"</LineString>\n"
			"</Placemark>\n"
			"</Document>\n"
			"</kml>\n"
			};
		strcat(main_buff, buff_3);

		fseek(f, pos, SEEK_SET);
		fprintf(f, "%s \n", main_buff);
	}
	fclose(f);
	return 1;
}
//---------------------------------------------------------------------------------------------------
uint8_t log_data_into_micro_sd(int name_of_file, float latitude, float longitude)
{
	static const char *SPI_FLASH = "SPI FLASH WRITE DATA:";
	const char* base_path = "/data";
	char name[20] = {0,};


	sprintf(name ,"%d.txt" , name_of_file);


	static int init = 0;
	if(init == 0)	// якщо перший раз запис у файл
	{
		ESP_ERROR_CHECK(example_mount_storage(base_path));

		char buff[600] = {
				"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
				"<Document>\n"
				"<name>simple route</name>\n"
		};

		char buff_2[100] = {
				"<Placemark>\n"
				"<name>Start point</name>\n"
				"<Point>\n"
				"<coordinates>"
		};

		strcat(buff, buff_2);

		char buf_lantitude[100] = {0,};
		// додати фейкові координати
		double lant = 24.01955;
		double lont = 49.80225;
		sprintf(buf_lantitude, "%.5f,%.5f</coordinates>\n"
				"</Point>\n"
				"</Placemark>\n",
				lant,lont);

		strcat(buff, buf_lantitude);

		ESP_LOGI(SPI_FLASH, "1111111: %s", buff);		// Fro debug

		char buff_3[200] = {
				"<Placemark>\n"
				"<name>Маршрут</name>\n"
				"<LineString>\n"
				"<coordinates>\n"
				"</coordinates>\n"
				"</LineString>\n"
				"</Placemark>\n"
				"</Document>\n"
				"</kml>\n"
		};

		strcat(buff, buff_3);

		ESP_LOGI(SPI_FLASH, "2222222: %s", buff);		// Debug

//		ESP_LOGI(TAG_LOG, "END 3: %s", buff);



		char mount_point_buf[30]= {0,};
		memset(mount_point_buf, 0, sizeof(mount_point_buf));
		strcat(mount_point_buf, base_path);

		if(LOG_INTO_COMPORT == ON)
		{
				ESP_LOGI(SPI_FLASH, "mount_point_buf:> %s", mount_point_buf);
		}

		strcat(mount_point_buf, "/");
		strcat(mount_point_buf, name);

		// ESP_LOGI(SPI_FLASH, "all mount_point_buf:> %s", mount_point_buf);
		FILE *f = fopen(mount_point_buf, "a");							// Відкрити або створити новий файл
		if (f == NULL)
		{
			ESP_LOGE(SPI_FLASH, "File can't be write !");
			//return;
			FILE *f = fopen(mount_point_buf, "w");							// Відкрити або створити новий файл
			if (f == NULL)
			{
				ESP_LOGE(SPI_FLASH, "File can't create log file !");
				return 0;
			}
			else
			{
				if(LOG_INTO_COMPORT == ON)
				{
					ESP_LOGI(SPI_FLASH, "New file was created !");
				}
			}
		}
		else
		{
//			if(LOG_INTO_COMPORT == ON)
//			{
//				ESP_LOGI(SPI_FLASH, "Write %s data", buff);
//			}
			ESP_LOGI(SPI_FLASH, "33333 Write %s data", buff);

			fprintf(f, "%s", buff);
			//fflush(f);
		}
		fclose(f);

		init = 1;

		return 0;
	}
	else
	{



//		char buff[200] = {
//				"111111111\n"
//				"222222222\n"
//				"333333333\n"
//		};
//		ESP_LOGI(SPI_FLASH, "ADD NEW DATA RAWS >>>>>>>>");
//		log_data111("/data", name, buff, "writing data");



		//////////////////////////////////////////////////////////////////////////////////////////////////////
 		char mount_point_buf[30]= {0,};
		strcat(mount_point_buf, base_path);

		if(LOG_INTO_COMPORT == ON)
		{
			ESP_LOGI(SPI_FLASH, "mount_point_buf:> %s", mount_point_buf);
		}

		strcat(mount_point_buf, "/");
		strcat(mount_point_buf, name);

		// ESP_LOGI(SPI_FLASH, "all mount_point_buf:> %s", mount_point_buf);

		FILE *f = fopen(mount_point_buf, "r+");							// Відкрити або створити новий файл
		if (f == NULL)
		{
			ESP_LOGE(SPI_FLASH, "File can't be write !");
			//return; ERROR
		}
		else
		{
			ESP_LOGI(SPI_FLASH, "Find end of file");

			// fprintf(f, "%s \n", data);


			// Перейти до кінця файлу
			if(fseek(f, 0, SEEK_END) != 0)
			{
				ESP_LOGE(SPI_FLASH, "ERROR on function fseek");
			}

			uint8_t roe_line_counter = 0;
			uint8_t target_row = 11;  // ДЛЯ ДЕБАГУ ....... МАЄ БУТИ 10 !!!!!!!!!!!
			//
			int debug_counter = 0;

			while(ftell(f) > 0)
			{
				fseek(f, -1, SEEK_CUR);			// Переміститися на символ файл назад

				char ch = fgetc(f);

				ESP_LOGI(SPI_FLASH, "CH: %c", ch);		// Debug

				if(ch == '\n')				// Знаходити символ нового рядка
				{
					ESP_LOGI(SPI_FLASH, "found new line': %d", roe_line_counter);

					roe_line_counter++;
					if(roe_line_counter == target_row)
					{
						break;
					}
				}
				fseek(f, -1, SEEK_CUR);

				// For debug
				ESP_LOGI(SPI_FLASH, "debug_counter: %d", debug_counter);
				debug_counter++;
			}

			long pos = ftell(f);		// Зберегти позицію для допису у файл

			// Перезаписати останні рядки <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

			char buff[400] = {0,};

			char buff_2[200] = {
				"<Placemark>\n"
				"<name>Start point</name>\n"
				"<Point>\n"
				"<coordinates>"
			};

			strcat(buff, buff_2);

			char buf_lantitude[80] = {0,};
			// додати фейкові координати
			double lant = 24.01955;
			double lont = 49.80235;
			sprintf(buf_lantitude, "%.5f,%.5f</coordinates>\n"
					"</Point>\n"
					"</Placemark>\n",
					lant,lont);

			strcat(buff, buf_lantitude);

			char buff_3[200] = {
					"<Placemark>\n"
					"<name>Маршрут</name>\n"
					"<LineString>\n"
					"<coordinates>\n"
					"</coordinates>\n"
					"</LineString>\n"
					"</Placemark>\n"
					"</Document>\n"
					"</kml>\n"
			};

			strcat(buff, buff_3);

			ESP_LOGI(SPI_FLASH, "3333333 >>>>>>>> %s", buff);

			fseek(f, pos, SEEK_SET);

			fprintf(f, "%s \n", buff);
			//fflush(f);
//			fclose(f);
		}
		fclose(f);


		ESP_LOGI(SPI_FLASH, "ALL FILE >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		f = fopen(mount_point_buf, "r+");
		char buffer_all_file[1000] = {0,};

		while(fgets(buffer_all_file, sizeof(buffer_all_file), f) != NULL)
		{
			ESP_LOGI(SPI_FLASH, "%s", buffer_all_file);
		}
		fclose(f);

		return 0;


		}
		return 0;
}
//---------------------------------------------------------------------------------------------------
int read_write_data_into_NVS(const char *key, uint8_t write_or_read, int data)
{
	static const char *MEM = "SAVE VALUE INTO NVS";
	static uint8_t init = 0;

	esp_err_t ret;
	nvs_handle_t my_handle;

	if(init == 0)
	{
		ret = nvs_flash_init();

		if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
		{
			ESP_ERROR_CHECK(nvs_flash_erase());
			ret = nvs_flash_init();
		}
		ESP_ERROR_CHECK(ret);
		init = 1;
	}

	if(write_or_read == write)	// Write variable
	{
		ret = nvs_open("storage", NVS_READWRITE, &my_handle);
		if (ret == ESP_OK)
		{
			ret = nvs_set_i32(my_handle, key, data);
			if (ret == ESP_OK)
			{
				ret = nvs_commit(my_handle);
			}
			nvs_close(my_handle);
		}
		ESP_ERROR_CHECK(ret);
	}

	else if(write_or_read == read)	// Read variable
	{
		int value = 0;
		ret = nvs_open("storage", NVS_READONLY, &my_handle);

		if (ret == ESP_OK)
		{
			ret = nvs_get_i32(my_handle, key, &value);
			nvs_close(my_handle);
		}
		ESP_ERROR_CHECK(ret);
		return value;
	}
	else
	{
		ESP_LOGE(MEM, "Wrong parametr");
	}

	return 0;
}
// --------------------------------------------------------------------------------------------------------------------
int get_file_name(char *name)
{
	static const char *MEM = "Log data: ";
	char name_of_file[15] = {0,};

	int value = read_write_data_into_NVS("name_of_file", read, 0);
	ESP_LOGI(MEM, "Read last file name: %d ", value);

	value = value + 1;
	sprintf(name_of_file, "%d", value);
	ESP_LOGI(MEM, "Current file name:: %s ", name_of_file);

	read_write_data_into_NVS("name_of_file", write, value);

	sprintf(name ,"%s.txt" , name_of_file);

	//ESP_LOGI(MEM, ">>>>>>>>>>FULL file name: %s ", name);

	return 0;
}
