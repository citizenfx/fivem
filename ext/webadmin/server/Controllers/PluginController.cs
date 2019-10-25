using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;

namespace FxWebAdmin
{
    public class PluginController : Controller
    {
        public async Task<IActionResult> Page(string name)
        {
            var stringRes = await BaseServer.Self.RunPluginController(name, HttpContext.Request.Query.ToDictionary(
                a => a.Key,
                a => (object)a.Value
            ), HttpContext);

            if (stringRes == "")
            {
                return NotFound();
            }

            return View((object)stringRes);
        }
    }
}