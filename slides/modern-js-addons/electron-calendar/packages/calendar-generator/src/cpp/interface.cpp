#include <napi.h>
#include <range/v3/view/concat.hpp>

#include <chrono>
#include <ranges>

import calendar;

Napi::Value generateCalendar(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() != 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::Error::New(env, "usage: generateCalendar(year: number, start_day: number)")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (info[1].As<Napi::Number>().Int32Value() < 0 || info[1].As<Napi::Number>().Int32Value() > 6) {
    Napi::Error::New(env, "start_day must be between 0 and 6")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  auto const year = info[0].As<Napi::Number>().Int32Value();
  auto const start_day = static_cast<std::chrono::weekday>(info[1].As<Napi::Number>().Int32Value());

  auto calendar = dates(std::chrono::year{year}, std::chrono::year{year + 1}) | format_calendar(start_day);
  Napi::Array result = Napi::Array::New(env, std::ranges::distance(calendar));
  for (auto [i, date] : std::views::enumerate(calendar))
  {
    result[static_cast<std::uint32_t>(i)] = Napi::String::New(env, date | std::ranges::to<std::string>());
  }
  return result;
}

static Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set("generateCalendar", Napi::Function::New(env, generateCalendar));
  return exports;
}

NODE_API_MODULE(calendar_generator, Init);
