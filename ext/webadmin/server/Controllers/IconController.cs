using System;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Net.Http.Headers;
using static CitizenFX.Core.Native.API;

namespace FxWebAdmin
{
    public class IconController : Controller
    {
        [AllowAnonymous]
        [HttpGet("~/icon.png")]
        public IActionResult GetIcon()
        {
            var iconString = GetConvar("sv_icon", "");

            if (string.IsNullOrWhiteSpace(iconString))
            {
                iconString = "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";
            }

            return File(
                Convert.FromBase64String(iconString),
                "image/png",
                DateTimeOffset.UtcNow - TimeSpan.FromHours(2),
                new EntityTagHeaderValue($"\"{GetHashKey(iconString).ToString("X")}\""),
                true);
        }
    }
}