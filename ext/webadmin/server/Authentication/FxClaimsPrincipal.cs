using System;
using System.Security.Claims;
using System.Security.Principal;
using AspNet.Security.OpenId.Steam;
using CitizenFX.Core.Native;

namespace FxWebAdmin
{
    internal class FxClaimsIdentity : ClaimsIdentity
    {
        public FxClaimsIdentity(IIdentity identity)
            : base(identity)
        {

        }

        public override bool HasClaim(string type, string value)
        {
            if (type == RoleClaimType)
            {
                var nameClaim = FindFirst(ClaimTypes.NameIdentifier);
                var coreIdentity = MapToIdentifier(nameClaim.Value);

                if (coreIdentity != null)
                {
                    if (API.IsPrincipalAceAllowed(coreIdentity, value))
                    {
                        return true;
                    }
                }
            }

            return base.HasClaim(type, value);
        }

        
        internal static string MapToIdentifier(string nameClaim)
        {
            if (nameClaim.StartsWith(SteamAuthenticationConstants.Namespaces.Identifier))
            {
                var identifier = ulong.Parse(nameClaim.Substring(SteamAuthenticationConstants.Namespaces.Identifier.Length));
                return $"identifier.steam:{identifier:x15}";
            }
            else if (nameClaim.StartsWith("https://forum.fivem.net/internal/user/"))
            {
                var identifier = nameClaim.Substring("https://forum.fivem.net/internal/user/".Length);
                return $"identifier.fivem:{identifier}";
            }
            else if (nameClaim == "https://cfx.re/id/rcon")
            {
                return "system.console";
            }

            return null;
        }
    }
}