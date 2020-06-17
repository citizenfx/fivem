using CitizenFX.Core.Native;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using System.Threading.Tasks;

namespace FxWebAdmin
{
    public class ConsoleController : Controller
    {
        public ConsoleController(ConsoleLog log)
        {

        }

        [Authorize(Roles = "webadmin.console.read")]
        public IActionResult Log()
        {
            return View(
                (object)API.GetConsoleBuffer()
            );
        }

        [Authorize(Roles = "webadmin.console.write")]
        public async Task<IActionResult> Execute([FromForm] string command)
        {
            await HttpServer.QueueTick(() =>
            {
                API.ExecuteCommand(command);
            });
            return RedirectToAction("Log");
        }
    }
}