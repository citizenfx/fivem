using System.Collections.Generic;
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
        private const string WebAdminRedirectUri = "/webadmin/";
        private const string RconPasswordClaimType = "cfx:rcon_password";
        private const string RconPasswordConvar = "rcon_password";
        private const string InvalidRconPasswordMessage = "Invalid RCON password.";

        [HttpGet]
        [AllowAnonymous]
        public IActionResult Login() => View();

        [HttpPost]
        [AllowAnonymous]
        public IActionResult LoginThirdParty([FromForm] string source)
        {
            if (source == "steam" || source == "discourse")
            {
                return Challenge(new AuthenticationProperties { RedirectUri = WebAdminRedirectUri }, source);
            }

            return Forbid();
        }

        [HttpPost]
        [AllowAnonymous]
        public async Task<IActionResult> LoginRcon([FromForm] string password)
        {
            var passwordVar = GetConvar(RconPasswordConvar, "");

            if (string.IsNullOrWhiteSpace(passwordVar) || password != passwordVar)
            {
                ViewBag.Error = InvalidRconPasswordMessage;
                return View("Login");
            }

            var claims = new List<Claim>
            {
                new Claim(ClaimTypes.Name, "System Console"),
                new Claim(ClaimTypes.NameIdentifier, "https://cfx.re/id/rcon"),
                new Claim(RconPasswordClaimType, password)
            };

            var claimsIdentity = new ClaimsIdentity(claims, CookieAuthenticationDefaults.AuthenticationScheme);

            await HttpContext.SignInAsync(
                CookieAuthenticationDefaults.AuthenticationScheme,
                new ClaimsPrincipal(claimsIdentity),
                new AuthenticationProperties { RedirectUri = WebAdminRedirectUri });

            return Redirect(WebAdminRedirectUri);
        }

        [HttpPost]
        public async Task<IActionResult> Logout()
        {
            await HttpContext.SignOutAsync(CookieAuthenticationDefaults.AuthenticationScheme);
            return Redirect(WebAdminRedirectUri);
        }
    }
}
