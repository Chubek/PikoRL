#ifndef PICORL_HPP
#define PICORL_HPP

#include "QaMRpp/QaMRpp.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace picorl
{

// The lpicorl extension that binds PicoRL features into the QaMRpp context
class PicoRLExtension : public qamrpp::Extension
{
public:
  void
  register_functions (qamrpp::Context &ctx) override
  {
    // Registering PicoRL core namespaces as a table conceptually,
    // or registering flat functions that populate 'lpicorl' table in Lua.

    // 1. Prompt keys (lpicorl.prompt)
    ctx.register_native (
        "lpicorl_prompt",
        [] (qamrpp::Context &c, std::vector<qamrpp::ValuePtr> &args)
          {
            std::cout << (args.size () > 0 ? args[0]->string_value : "> ")
                      << std::flush;
            std::string line;
            std::getline (std::cin, line);
            auto val = std::make_shared<qamrpp::Value> ();
            val->type = qamrpp::Value::Type::STRING;
            val->string_value = line;
            return val;
          });

    // 2. Syntax highlighting (lpicorl.syntax)
    ctx.register_native (
        "lpicorl_syntax",
        [] (qamrpp::Context &c, std::vector<qamrpp::ValuePtr> &args)
          {
            // Stub for syntax highlighting integration
            return std::make_shared<qamrpp::Value> ();
          });

    // 3. Directive commands (lpicorl.directive)
    ctx.register_native (
        "lpicorl_directive",
        [] (qamrpp::Context &c, std::vector<qamrpp::ValuePtr> &args)
          {
            // Stub for REPL directives (e.g., %load, %clear)
            return std::make_shared<qamrpp::Value> ();
          });

    // 4. Shared library plugins (lpicorl.plugin)
    ctx.register_native (
        "lpicorl_plugin",
        [] (qamrpp::Context &c, std::vector<qamrpp::ValuePtr> &args)
          {
            if (!args.empty () && args[0]->type == qamrpp::Value::Type::STRING)
              {
                bool ok = c.load_library (args[0]->string_value);
                auto res = std::make_shared<qamrpp::Value> ();
                res->type = qamrpp::Value::Type::BOOL;
                res->bool_value = ok;
                return res;
              }
            return std::make_shared<qamrpp::Value> ();
          });

    // Other stubs for menus, web, sandboxing, IDL, themes, macros...
    // lpicorl.page, lpicorl.menue, lpicorl.web, lpicorl.sandboxing
    // lpicorl.idl, lpicorl.them, lpicorl.colors, lpicorl.macros
  }
};

// The Main REPL Toolkit Class
class REPL
{
private:
  qamrpp::Context ctx;
  bool is_running;

public:
  REPL () : is_running (false)
  {
    // Load QaMRpp Standard libraries
    qamrpp::stdlib::load_core (ctx);
    qamrpp::stdlib::load_string (ctx);
    qamrpp::stdlib::load_table (ctx);
    qamrpp::stdlib::load_math (ctx);
    qamrpp::stdlib::load_io (ctx);
    qamrpp::stdlib::load_os (ctx);
    qamrpp::stdlib::load_package (ctx);

    // Add the PicoRL Extension
    ctx.add_extension (std::make_unique<PicoRLExtension> ());

    // Bootstrap the lpicorl table in Lua space
    // (Assuming a simple script to map the natives to the lpicorl table)
    // ctx.execute("lpicorl = { prompt = lpicorl_prompt, plugin =
    // lpicorl_plugin, syntax = lpicorl_syntax, directive = lpicorl_directive
    // }");
  }

  void
  run ()
  {
    is_running = true;
    std::cout << "PicoRL - Header-Only REPL Toolkit\n";
    std::cout << "Powered by QaMRpp Lua\n";

    while (is_running)
      {
        // Use the lpicorl prompt natively or via lua script
        std::cout << "PicoRL> " << std::flush;
        std::string input;
        if (!std::getline (std::cin, input))
          {
            break;
          }

        if (input == "exit" || input == "quit")
          {
            is_running = false;
            break;
          }

        try
          {
            // Here we would parse and execute `input` using QaMRpp's execution
            // interface e.g., ctx.execute(input);
          }
        catch (const std::exception &e)
          {
            std::cerr << "Error: " << e.what () << "\n";
          }
      }
  }

  // Allow host language to inject its own APIs/Globals into the REPL
  template <typename F>
  void
  bind_api (const std::string &name, F function)
  {
    ctx.register_native (name, function);
  }
};

} // namespace picorl

#endif // PICORL_HPP
