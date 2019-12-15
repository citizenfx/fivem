using CitizenFX.Core.Native;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

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
    }
}