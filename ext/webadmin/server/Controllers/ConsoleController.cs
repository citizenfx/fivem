using CitizenFX.Core.Native;

using System.Threading.Tasks;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;


namespace FxWebAdmin
{
    public class ConsoleController : Controller
    {
        [Authorize(Roles = "webadmin.console.read")]
        public IActionResult Log()
        {
            return View(
                (object)API.GetConsoleBuffer()
            );
        }
        public async Task<IActionResult> Execute([FromForm] string command)
        {
            await HttpServer.QueueTick(() =>
            {
                // TODO: access control for this(!)
                API.ExecuteCommand(command);
            });
            return RedirectToAction("Log");
        }
    }
}