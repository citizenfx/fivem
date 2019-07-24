using System.Security.Claims;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Authentication;

namespace FxWebAdmin
{
    internal class FxClaimsTransformer : IClaimsTransformation
    {
        public Task<ClaimsPrincipal> TransformAsync(ClaimsPrincipal principal)
        {
            return Task.FromResult(new ClaimsPrincipal(new FxClaimsIdentity(principal.Identity)));
        }
    }
}