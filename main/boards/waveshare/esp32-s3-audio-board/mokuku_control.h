#pragma once

#include <esp_log.h>
#include <wifi_manager.h>
#include "esp_now.h"
#include "mcp_server.h"
#include "mokuku_control_header.h"
#include "wifi_board.h"

#define TAG_T "[MKK]"

// Model Context Protocol
class MokukuControl {
 private:
  TaskHandle_t esp_now_task_handle_ = nullptr;
  uint8_t latest_emoji_id_ = 0;

 public:
  static MokukuControl& GetInstance() {
    static MokukuControl instance;
    return instance;
  }
  void set_up_emoji(uint8_t id, bool force) {
    if (latest_emoji_id_ > 0 && !force) {
      return;
    }
    latest_emoji_id_ = id;
  }

  MokukuControl() {
    auto& mcp_server = McpServer::GetInstance();
    mcp_server.AddTool("self.mokuku.set_emoji",
                      "preform emojis. emoji: the name of the emotion; id: the id of the emtion; "
                      "You can display the following expressions (the IDs are shown in parentheses)."
                      "happy(id:38), blink(id:28),"
                      "dizzy(id:5), sleeping(id:8), bored(id:27),"
                      "pitiful(id:43), starry eyes(id:40), bored(id:27), cute(id:31), cute(id:54),"
                      "bubble pop(id:33), basketball(id:52), snake game(id:50), jump rope(id:44),",
                      PropertyList({
                           Property("emoji", kPropertyTypeString),
                           Property("id", kPropertyTypeInteger, 0, 0, 100),
                       }),
                       [this](const PropertyList& properties) -> ReturnValue {
                         const std::string& action = properties["emoji"].value<std::string>();
                         int emotion_id = properties["id"].value<int>();
                         if (emotion_id > 0 && emotion_id < 100) {
                           latest_emoji_id_ = emotion_id;
                         }
                         ESP_LOGI(TAG_T, "received emoji %s %d", action.c_str(), emotion_id);
                         return true;
                       });

    mcp_server.AddTool("self.mokuku.volume",
                       "change the volume of the speaker (0~100)",
                       PropertyList({
                           Property("volume", kPropertyTypeInteger, 80, 0, 100),
                       }),
                       [this](const PropertyList& properties) -> ReturnValue {
                         int volume = properties["volume"].value<int>();

                         auto& board = Board::GetInstance();
                         board.GetAudioCodec()->SetOutputVolume(volume);
                         ESP_LOGI(TAG_T, "change volume %d", volume);
                         return true;
                       });

    // TODO obtain the status of the car from MOKUKU

    xTaskCreatePinnedToCore(
        [](void* arg) {
          MokukuControl* mkk_control = (MokukuControl*)arg;
          mkk_control->create_esp_now_server();
          vTaskDelete(NULL);
        },
        "mkk_esp_now", 2048 * 3, this, 8, &esp_now_task_handle_, 0);
  }

  void create_esp_now_server() {
    // wait until wifi to be connected
    auto& wifi = WifiManager::GetInstance();
    while (!wifi.IsConnected()) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG_T, "create ble client");
    gattc_client_main();

    bool idling_setup = false;
    uint8_t command_idling[] = {1, 20};
    uint8_t command_emoji[] = {1, 0};

    while (true) {
      if (!idling_setup) {
        if (traverse_send_peer(sizeof(command_idling), command_idling)) {
          idling_setup = true;
          ESP_LOGI(TAG_T, "idling setup done");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
      }

      // obtian the meme to show
      if (latest_emoji_id_ > 0) {
        command_emoji[1] = latest_emoji_id_ + 100;
        if (traverse_send_peer(sizeof(command_emoji), command_emoji)) {
          ESP_LOGI(TAG_T, "set emoji done %d", latest_emoji_id_);
          latest_emoji_id_ = 0;
        }
      } else {
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
};
