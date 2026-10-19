#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <cmath>
#include <string>

using namespace std::chrono_literals;

#include <cpprest/http_msg.h>
#include <wiringPi.h>

#include "metronome.hpp"
#include "rest.hpp"

// ** Remember to update these numbers to your personal setup. **
#define LED_RED   17
#define LED_GREEN 27
#define BTN_MODE  23
#define BTN_TAP   24

// Metronome engine instance
static metronome g_metronome;

// State management (thread-safe)
static std::mutex g_state_mutex;
static std::atomic<size_t> g_current_bpm(0);
static std::atomic<size_t> g_min_bpm(0);
static std::atomic<size_t> g_max_bpm(0);
static std::atomic<bool> g_is_learn_mode(false);
static std::atomic<bool> g_red_pulse(false);
static std::chrono::steady_clock::time_point g_pulse_time;

// Helper to set current BPM and update min/max statistics
void set_bpm(size_t bpm) {
	std::lock_guard<std::mutex> lock(g_state_mutex);
	g_current_bpm = bpm;

	// Per spec: min tracks lowest BPM the user set (not 0)
	if (bpm > 0) {
		if (g_min_bpm == 0 || bpm < g_min_bpm) {
			g_min_bpm = bpm;
		}
		if (g_max_bpm == 0 || bpm > g_max_bpm) {
			g_max_bpm = bpm;
		}
	}
}

// REST handler: GET /bpm/
void handle_get_bpm(web::http::http_request msg) {
	std::cout << "[REST] GET /bpm/ -> " << g_current_bpm.load() << std::endl;
	rest::reply_json(msg, 200, web::json::value::number(g_current_bpm.load()));
}

// REST handler: PUT /bpm/
void handle_put_bpm(web::http::http_request msg) {
	msg.extract_json().then([msg](pplx::task<web::json::value> task) {
		try {
			web::json::value body = task.get();
			size_t new_bpm = 0;

			if (body.is_integer()) {
				new_bpm = static_cast<size_t>(body.as_integer());
			} else if (body.is_double()) {
				new_bpm = static_cast<size_t>(body.as_double());
			} else if (body.is_object() && body.has_field(U("bpm"))) {
				auto val = body.at(U("bpm"));
				if (val.is_integer()) {
					new_bpm = static_cast<size_t>(val.as_integer());
				} else if (val.is_double()) {
					new_bpm = static_cast<size_t>(val.as_double());
				} else if (val.is_string()) {
					new_bpm = std::stoul(val.as_string());
				}
			} else if (body.is_string()) {
				new_bpm = std::stoul(body.as_string());
			}

			set_bpm(new_bpm);
			std::cout << "[REST] PUT /bpm/ -> " << new_bpm
			          << " (Min: " << g_min_bpm.load()
			          << ", Max: " << g_max_bpm.load() << ")" << std::endl;

			rest::reply_json(msg, 200, web::json::value::number(g_current_bpm.load()));
		} catch (...) {
			std::cerr << "[REST ERROR] Invalid PUT /bpm/ payload" << std::endl;
			rest::reply_json(msg, 400, web::json::value::string(U("Invalid BPM value")));
		}
	});
}

// REST handler: GET /bpm/min/
void handle_get_min(web::http::http_request msg) {
	std::cout << "[REST] GET /bpm/min/ -> " << g_min_bpm.load() << std::endl;
	rest::reply_json(msg, 200, web::json::value::number(g_min_bpm.load()));
}

// REST handler: DELETE /bpm/min/
void handle_delete_min(web::http::http_request msg) {
	std::lock_guard<std::mutex> lock(g_state_mutex);
	g_min_bpm = 0;
	std::cout << "[REST] DELETE /bpm/min/ -> reset to 0" << std::endl;
	rest::reply_json(msg, 200, web::json::value::number(0));
}

// REST handler: GET /bpm/max/
void handle_get_max(web::http::http_request msg) {
	std::cout << "[REST] GET /bpm/max/ -> " << g_max_bpm.load() << std::endl;
	rest::reply_json(msg, 200, web::json::value::number(g_max_bpm.load()));
}

// REST handler: DELETE /bpm/max/
void handle_delete_max(web::http::http_request msg) {
	std::lock_guard<std::mutex> lock(g_state_mutex);
	g_max_bpm = 0;
	std::cout << "[REST] DELETE /bpm/max/ -> reset to 0" << std::endl;
	rest::reply_json(msg, 200, web::json::value::number(0));
}

// Central Request Dispatcher (Handles /bpm, /bpm/min, /bpm/max with/without trailing slash and CORS)
void handle_request(web::http::http_request msg) {
	auto method = msg.method();
	std::string path = msg.relative_uri().path();

	// Normalise path (remove trailing slash except for root "/")
	std::string norm_path = path;
	while (norm_path.length() > 1 && norm_path.back() == '/') {
		norm_path.pop_back();
	}

	// Handle CORS preflight
	if (method == web::http::methods::OPTIONS) {
		rest::allowAll(msg);
		return;
	}

	// Root / Information
	if (norm_path.empty() || norm_path == "/") {
		web::json::value info;
		info[U("service")] = web::json::value::string(U("Metronome Thing REST API"));
		info[U("status")] = web::json::value::string(U("online"));
		info[U("bpm")] = web::json::value::number(g_current_bpm.load());
		info[U("min")] = web::json::value::number(g_min_bpm.load());
		info[U("max")] = web::json::value::number(g_max_bpm.load());
		rest::reply_json(msg, 200, info);
		return;
	}

	// Route: /bpm
	if (norm_path == "/bpm") {
		if (method == web::http::methods::GET) {
			handle_get_bpm(msg);
		} else if (method == web::http::methods::PUT) {
			handle_put_bpm(msg);
		} else {
			msg.reply(405);
		}
		return;
	}

	// Route: /bpm/min
	if (norm_path == "/bpm/min") {
		if (method == web::http::methods::GET) {
			handle_get_min(msg);
		} else if (method == web::http::methods::DEL) {
			handle_delete_min(msg);
		} else {
			msg.reply(405);
		}
		return;
	}

	// Route: /bpm/max
	if (norm_path == "/bpm/max") {
		if (method == web::http::methods::GET) {
			handle_get_max(msg);
		} else if (method == web::http::methods::DEL) {
			handle_delete_max(msg);
		} else {
			msg.reply(405);
		}
		return;
	}

	// Not Found
	std::cout << "[REST] 404 Not Found: " << path << std::endl;
	rest::reply_json(msg, 404, web::json::value::string(U("Endpoint not found")));
}

// Separate thread managing LED behavior
void blink() {
	bool green_on = false;
	auto last_toggle = std::chrono::steady_clock::now();

	while (true) {
		auto now = std::chrono::steady_clock::now();

		if (g_is_learn_mode.load()) {
			// In LEARN mode: Green LED is always OFF
			if (green_on) {
				green_on = false;
				digitalWrite(LED_GREEN, LOW);
			}

			// Red LED pulses momentarily (100ms) on each tap
			if (g_red_pulse.load()) {
				auto elapsed_pulse = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_pulse_time).count();
				if (elapsed_pulse < 100) {
					digitalWrite(LED_RED, HIGH);
				} else {
					digitalWrite(LED_RED, LOW);
					g_red_pulse = false;
				}
			} else {
				digitalWrite(LED_RED, LOW);
			}
		} else {
			// In PLAY mode: Red LED is always OFF
			digitalWrite(LED_RED, LOW);
			g_red_pulse = false;

			size_t bpm = g_current_bpm.load();
			if (bpm == 0) {
				// Default / stopped tempo: Green LED is OFF
				if (green_on) {
					green_on = false;
					digitalWrite(LED_GREEN, LOW);
				}
			} else {
				// 50% duty cycle blink: period = 60000 / BPM ms; half period = 30000 / BPM ms
				double half_period_ms = 30000.0 / static_cast<double>(bpm);
				auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_toggle).count();

				if (elapsed >= half_period_ms) {
					green_on = !green_on;
					digitalWrite(LED_GREEN, green_on ? HIGH : LOW);
					last_toggle = now;
				}
			}
		}

		// Responsive sleep slice
		std::this_thread::sleep_for(10ms);
	}
}

int main() {
	// 1. Initialize WiringPi using Broadcom (BCM) GPIO numbering
	bool gpio_available = true;
	if (wiringPiSetupGpio() < 0) {
		std::cerr << "--------------------------------------------------------" << std::endl;
		std::cerr << "[WARNING] wiringPiSetupGpio() failed to access GPIO!" << std::endl;
		std::cerr << "          To enable physical LEDs & buttons, run with: sudo ./main" << std::endl;
		std::cerr << "          Starting in simulation mode (REST API fully active)..." << std::endl;
		std::cerr << "--------------------------------------------------------" << std::endl;
		gpio_available = false;
	} else {
		// 2. Set pin modes
		pinMode(LED_RED, OUTPUT);
		pinMode(LED_GREEN, OUTPUT);
		pinMode(BTN_MODE, INPUT);
		pinMode(BTN_TAP, INPUT);

		// Enable internal pull-down resistors (pressed = HIGH)
		pullUpDnControl(BTN_MODE, PUD_DOWN);
		pullUpDnControl(BTN_TAP, PUD_DOWN);

		// Ensure initial LED states are OFF
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_GREEN, LOW);
	}

	// 3. Configure and start REST Server on port 8080
	http_listener listener;
	try {
		listener = rest::make_endpoint("");
		listener.support(web::http::methods::GET, handle_request);
		listener.support(web::http::methods::PUT, handle_request);
		listener.support(web::http::methods::DEL, handle_request);
		listener.support(web::http::methods::OPTIONS, handle_request);
		listener.open().wait();

		std::cout << "==========================================" << std::endl;
		std::cout << " Metronome Thing Service is Running!     " << std::endl;
		std::cout << " Port: 8080 (http://0.0.0.0:8080/)       " << std::endl;
		std::cout << " Endpoints: /bpm/, /bpm/min/, /bpm/max/  " << std::endl;
		std::cout << " Hardware GPIO: " << (gpio_available ? "ACTIVE" : "INACTIVE (Simulation)") << std::endl;
		std::cout << " Mode: PLAY (default BPM = 0, LED OFF)   " << std::endl;
		std::cout << "==========================================" << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "[FATAL ERROR] Failed to bind HTTP listener on port 8080: " << e.what() << std::endl;
		std::cerr << "Check if port 8080 is in use: sudo ss -tulpn | grep 8080" << std::endl;
		return 1;
	}

	// 4. Start LED blinking thread (if GPIO available)
	if (gpio_available) {
		std::thread blink_thread(blink);
		blink_thread.detach();

		// 5. Button polling loop with rising-edge detection and debouncing
		bool mode_prev = false;
		bool tap_prev = false;
		auto last_mode_press = std::chrono::steady_clock::now();
		auto last_tap_press = std::chrono::steady_clock::now();
		const auto debounce_time = 50ms;

		while (true) {
			auto now = std::chrono::steady_clock::now();
			bool mode_curr = (digitalRead(BTN_MODE) == HIGH);
			bool tap_curr = (digitalRead(BTN_TAP) == HIGH);

			// Rising edge on Mode button (toggle Play / Learn)
			if (mode_curr && !mode_prev && (now - last_mode_press >= debounce_time)) {
				last_mode_press = now;

				if (!g_is_learn_mode.load()) {
					// Transition PLAY -> LEARN
					std::cout << "\n>>> Entering LEARN mode. Tap BTN_TAP (PB1) at least 4 times to set tempo..." << std::endl;
					g_is_learn_mode = true;
					g_metronome.start_timing();
				} else {
					// Transition LEARN -> PLAY
					g_metronome.stop_timing();
					g_is_learn_mode = false;
					size_t learned_bpm = g_metronome.get_bpm();

					std::cout << "\n>>> Leaving LEARN mode. Calculating tempo..." << std::endl;
					if (learned_bpm > 0) {
						set_bpm(learned_bpm);
						std::cout << ">>> Valid BPM learned: " << learned_bpm << " BPM!" << std::endl;
						std::cout << ">>> Stats: Current = " << g_current_bpm.load()
						          << ", Min = " << g_min_bpm.load()
						          << ", Max = " << g_max_bpm.load() << std::endl;
					} else {
						std::cout << ">>> Not enough taps (< 4). Keeping previous BPM: "
						          << g_current_bpm.load() << " BPM." << std::endl;
					}
				}
			}

			// Rising edge on Tap button (records tap in Learn mode)
			if (tap_curr && !tap_prev && (now - last_tap_press >= debounce_time)) {
				last_tap_press = now;

				if (g_is_learn_mode.load()) {
					g_metronome.tap();
					g_pulse_time = now;
					g_red_pulse = true;
					std::cout << "  * Tap recorded! (Red LED pulse)" << std::endl;
				}
			}

			mode_prev = mode_curr;
			tap_prev = tap_curr;

			std::this_thread::sleep_for(10ms);
		}
	} else {
		// Keep server alive in simulation mode
		while (true) {
			std::this_thread::sleep_for(1s);
		}
	}

	return 0;
}
