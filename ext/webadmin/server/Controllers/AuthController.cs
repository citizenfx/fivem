using System.Collections.Generic;
using System.Dynamic;
using System.Security.Claims;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Authentication;
using Microsoft.AspNetCore.Authentication.Cookies;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

using static CitizenFX.Core.Native.API;

namespace FxWebAdmin
{
    public class AuthController : Controller
    {
        [HttpGet]
        [AllowAnonymous]
        public IActionResult Login()
        {
            return View();
        }

        [HttpPost]
        [AllowAnonymous]
        public IActionResult LoginThirdParty([FromForm] string source)
        {
            if (source == "steam" || source == "discourse")
            {
                return Challenge(new AuthenticationProperties()
                {
                    RedirectUri = "/webadmin/"
                }, source);
            }

            return Forbid();
        }

        [HttpPost]
        [AllowAnonymous]
        public async Task<IActionResult> LoginRcon([FromForm] string password)
        {
            var passwordVar = GetConvar("rcon_password", "");

            if (string.IsNullOrWhiteSpace(passwordVar))
            {
                ViewBag.Error = "Invalid RCON password.";

                return View("Login");
            }

            if (password != passwordVar)
            {
                ViewBag.Error = "Invalid RCON password.";

                return View("Login");
            }

            var claims = new List<Claim>
            {
                new Claim(ClaimTypes.Name, "System Console"),
                new Claim(ClaimTypes.NameIdentifier, "https://cfx.re/id/rcon"),
                new Claim("cfx:rcon_password", password)
            };

            var claimsIdentity = new ClaimsIdentity(claims, CookieAuthenticationDefaults.AuthenticationScheme);

            var authProperties = new AuthenticationProperties()
            {
                RedirectUri = "/webadmin/"
            };
            
            await HttpContext.SignInAsync(
                CookieAuthenticationDefaults.AuthenticationScheme,
                new ClaimsPrincipal(claimsIdentity),
                authProperties);

            return Redirect("/webadmin/");
        }

        [HttpPost]
        public async Task<IActionResult> Logout()
        {
            await HttpContext.SignOutAsync(new AuthenticationProperties());

            return Redirect("/webadmin/");
        }
    }
}