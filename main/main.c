#include <stdio.h>
#include "connect.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "mdns.h"
#include "togglePin.h"
#include "cJSON.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
// #include "pushBtn.h"

static const char *BASE_PATH = "/store";

static const char *TAG = "SERVER";

static httpd_handle_t server = NULL;

static esp_err_t on_default_url(httpd_req_t *req)
{
  ESP_LOGI(TAG, "URL: %s", req->uri);
  char path[600];
  sprintf(path,"/store%s",req->uri);

  char *ext = strrchr(req->uri,'.');
  if(ext)
  {
    if(strcmp(ext,".css") == 0) httpd_resp_set_type(req,"text/css");
    if(strcmp(ext,".js") == 0) httpd_resp_set_type(req,"text/javascript");
    if(strcmp(ext,".png") == 0) httpd_resp_set_type(req,"image/png");
    if(strcmp(ext,".jpg") == 0) httpd_resp_set_type(req,"image/jpg");
    if(strcmp(ext,".svg") == 0) httpd_resp_set_type(req,"image/svg+xml");
  }

  FILE *file = fopen(path,"r");
  if(file == NULL)
  {
    file = fopen("/store/index.html","r");
    if(file == NULL) {
      httpd_resp_send_404(req);
    }

  }
  char buffer[1024];
  int bytes_read = 0;
  while ((bytes_read = fread(buffer,sizeof(char),sizeof(buffer),file)) > 0)
  {
    httpd_resp_send_chunk(req,buffer,bytes_read);
  }
  fclose(file);

  httpd_resp_send_chunk(req,NULL,0);

  return ESP_OK;
}
// static esp_err_t on_toggle_led_url(httpd_req_t *req)
// {
//   char buffer[100];
//   memset(&buffer, 0, sizeof(buffer));
//   httpd_req_recv(req, buffer, req->content_len);
//   cJSON *payload = cJSON_Parse(buffer);
//   cJSON *is_on_json = cJSON_GetObjectItem(payload, "is_on");
//   bool is_on = cJSON_IsTrue(is_on_json);
//   cJSON_Delete(payload);
//   toggle_led(is_on);
//   httpd_resp_set_status(req, "204 NO CONTENT");
//   httpd_resp_send(req, NULL, 0);
//   return ESP_OK;
// }
/********************Web Socket *******************/

#define WS_MAX_SIZE 1024
static int client_session_id;

esp_err_t send_ws_message(char *message)
{
  if (!client_session_id)
  {
    ESP_LOGE(TAG, "no client_session_id");
    return -1;
  }
  httpd_ws_frame_t ws_message = {
      .final = true,
      .fragmented = false,
      .len = strlen(message),
      .payload = (uint8_t *)message,
      .type = HTTPD_WS_TYPE_TEXT};
  return httpd_ws_send_frame_async(server, client_session_id, &ws_message);
}

static esp_err_t on_web_socket_url(httpd_req_t *req)
{
  client_session_id = httpd_req_to_sockfd(req);
  if (req->method == HTTP_GET)
    return ESP_OK;

  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;
  ws_pkt.payload = malloc(WS_MAX_SIZE);
  httpd_ws_recv_frame(req, &ws_pkt, WS_MAX_SIZE);
  printf("ws payload: %.*s\n", ws_pkt.len, ws_pkt.payload);

    // Parse JSON
    cJSON *json = cJSON_ParseWithLength((const char *)ws_pkt.payload, ws_pkt.len);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "JSON parse error: %s\n", error_ptr);
        }
        free(ws_pkt.payload);
        return ESP_FAIL;
    }

    // Process JSON (example: get values from JSON)
    cJSON *left = cJSON_GetObjectItemCaseSensitive(json, "left");
    cJSON *right = cJSON_GetObjectItemCaseSensitive(json, "right");

    if (cJSON_IsNumber(left) && cJSON_IsNumber(right)) {
        printf("left: %d, right: %d\n", left->valueint, right->valueint);

        toggle_pin("left", left->valueint);
        toggle_pin("right", right->valueint);

    } else {
        printf("Invalid JSON format\n");
    
    }

    // Clean up
    cJSON_Delete(json);




  free(ws_pkt.payload);






  char *response = "connected OK 😊";
  httpd_ws_frame_t ws_responce = {
      .final = true,
      .fragmented = false,
      .type = HTTPD_WS_TYPE_TEXT,
      .payload = (uint8_t *)response,
      .len = strlen(response)};
  return httpd_ws_send_frame(req, &ws_responce);
}

/*******************************************/

static void init_server()
{

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.uri_match_fn = httpd_uri_match_wildcard;

  ESP_ERROR_CHECK(httpd_start(&server, &config));

  // httpd_uri_t default_url = {
  //     .uri = "/",
  //     .method = HTTP_GET,
  //     .handler = on_default_url};
  // httpd_register_uri_handler(server, &default_url);

  // httpd_uri_t toggle_led_url = {
  //     .uri = "/api/toggle-led",
  //     .method = HTTP_POST,
  //     .handler = on_toggle_led_url};
  // httpd_register_uri_handler(server, &toggle_led_url);

  httpd_uri_t web_socket_url = {
      .uri = "/ws",
      .method = HTTP_GET,
      .handler = on_web_socket_url,
      .is_websocket = true};
  httpd_register_uri_handler(server, &web_socket_url);

  httpd_uri_t default_url = {
      .uri = "/*",
      .method = HTTP_GET,
      .handler = on_default_url};
  httpd_register_uri_handler(server, &default_url);
}

void start_mdns_service()
{
  mdns_init();
  mdns_hostname_set("my-esp32");
  mdns_instance_name_set("LEARN esp32 thing");
}


void mount_fs(void)
{
    esp_vfs_fat_mount_config_t esp_vfs_fat_mount_config = {
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE,
        .max_files = 5,
        .format_if_mount_failed = true,
    };

    esp_vfs_fat_spiflash_mount_ro(BASE_PATH, "storage", &esp_vfs_fat_mount_config);
}

void app_main(void)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  // init_led();
  // init_btn();
  wifi_init();
  ESP_ERROR_CHECK(wifi_connect_sta("Midget cannon", "BadassTruck1", 10000));

    // ESP_ERROR_CHECK(wifi_connect_sta("iPhone", "12345678", 10000));


  start_mdns_service();
  mount_fs();
  init_server();

  init_pin();


}