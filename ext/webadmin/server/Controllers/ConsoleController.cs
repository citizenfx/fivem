using System.Collections.Generic;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using static CitizenFX.Core.Native.API;

namespace FxWebAdmin
{
    public class ConsoleController : Controller
    {
        [Authorize(Roles = "webadmin.console.read")]
        public IActionResult Log()
        {
            return View(
                (object)GetConsoleBuffer()
            );
        }
        public async Task<IActionResult> Execute([FromForm] string command)
        {
            await HttpServer.QueueTick(() =>
            {
                // TODO: access control for this(!)
                ExecuteCommand(command);
            });

            return RedirectToAction("Log");
        }
    }
}