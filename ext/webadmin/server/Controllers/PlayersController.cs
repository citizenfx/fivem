using System.Security.Claims;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using static CitizenFX.Core.Native.API;

namespace FxWebAdmin
{
    public class PlayersController : Controller
    {
        [HttpPost]
        [Authorize(Roles = "command.clientkick")]
        public IActionResult Kick([FromForm] string source)
        {
            DropPlayer(source, "Kicked from web administration by " + User?.FindFirst(ClaimTypes.Name)?.Value ?? "[unknown admin]");

            return RedirectToAction("Index", "Home");
        }
    }
}