using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Security.Claims;
using System.Security.Cryptography;
using System.Text;
using System.Text.Encodings.Web;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Authentication;
using Microsoft.AspNetCore.Authentication.Internal;
using Microsoft.AspNetCore.DataProtection;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.WebUtilities;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Options;
using Newtonsoft.Json.Linq;

namespace FxWebAdmin.Authentication
{
    public class DiscourseAuthenticationHandler : RemoteAuthenticationHandler<DiscourseAuthenticationOptions>
    {
        private const string NonceProperty = "N";

        private HttpClient _httpClient;

        public DiscourseAuthenticationHandler(IOptionsMonitor<DiscourseAuthenticationOptions> options, ILoggerFactory logger, UrlEncoder encoder, ISystemClock clock) : base(options, logger, encoder, clock)
        {
            _httpClient = new HttpClient();
        }

        protected override async Task<HandleRequestResult> HandleRemoteAuthenticateAsync()
        {
            if (!string.Equals(Request.Method, "GET", StringComparison.OrdinalIgnoreCase))
            {
                return HandleRequestResult.Fail("This needs to be a GET request.");
            }

            var query = QueryHelpers.ParseQuery(Request.QueryString.Value.Replace("?payload=", "&payload="));

            var state = query["state"];
            
            if (string.IsNullOrEmpty(state))
            {
                return HandleRequestResult.Fail("State missing.");
            }

            var payload = query["payload"];

            if (string.IsNullOrEmpty(payload))
            {
                return HandleRequestResult.Fail("Payload missing.");
            }

            var properties = Options.StateDataFormat.Unprotect(state);

            if (properties == null)
            {
                return HandleRequestResult.Fail("The authentication response was rejected " +
                                                "because the state parameter was invalid.");
            }

            var payloadBytes = Convert.FromBase64String(payload);
            JObject json;
            
            using (var rsa = RSA.Create())
            {
                rsa.ImportParameters(Options.RSAKey);

                var decryptedPayload = rsa.Decrypt(payloadBytes, RSAEncryptionPadding.Pkcs1);
                json = JObject.Parse(Encoding.UTF8.GetString(decryptedPayload));
            }

            var nonce = ReadNonceCookie(json.Value<string>("nonce"));

            if (nonce == null)
            {
                return HandleRequestResult.Fail("Invalid nonce.");
            }

            var authToken = json.Value<string>("key");

            var response = await _httpClient.SendAsync(new HttpRequestMessage(HttpMethod.Get, $"{Options.DiscourseUri}session/current.json")
            {
                Headers =
                {
                    { "User-Api-Client-Id", Options.ClientId },
                    { "User-Api-Key", authToken },
                }
            });

            if (!response.IsSuccessStatusCode)
            {
                return HandleRequestResult.Fail("Could not get current session.");
            }

            var responseBody = JObject.Parse(await response.Content.ReadAsStringAsync());

            var avatarTemplate = responseBody["current_user"].Value<string>("avatar_template");

            if (!avatarTemplate.Contains("://"))
            {
                avatarTemplate = Options.DiscourseUri + avatarTemplate;
            }

            var identity = new ClaimsIdentity(Scheme.Name);
            identity.AddClaim(new Claim(ClaimTypes.NameIdentifier, Options.DiscourseUri + "internal/user/" + responseBody["current_user"].Value<int>("id"), ClaimValueTypes.String, Options.ClaimsIssuer));
            identity.AddClaim(new Claim(ClaimTypes.Name, responseBody["current_user"].Value<string>("username"), ClaimValueTypes.String, Options.ClaimsIssuer));
            identity.AddClaim(new Claim("cfx:avatar", avatarTemplate.Replace("{size}", "128"), ClaimValueTypes.String, Options.ClaimsIssuer));

            var principal = new ClaimsPrincipal(identity);
            properties.StoreTokens(new AuthenticationToken[] { new AuthenticationToken() { Name = "key", Value = authToken } });

            var ticket = new AuthenticationTicket(principal, properties, Scheme.Name);

            return HandleRequestResult.Success(ticket);
        }

        /// <summary>
        /// Searches <see cref="HttpRequest.Cookies"/> for a matching nonce.
        /// </summary>
        /// <param name="nonce">the nonce that we are looking for.</param>
        /// <returns>echos 'nonce' if a cookie is found that matches, null otherwise.</returns>
        /// <remarks>Examine <see cref="IRequestCookieCollection.Keys"/> of <see cref="HttpRequest.Cookies"/> that start with the prefix: 'OpenIdConnectAuthenticationDefaults.Nonce'.
        /// <see cref="M:ISecureDataFormat{TData}.Unprotect"/> of <see cref="OpenIdConnectOptions.StringDataFormat"/> is used to obtain the actual 'nonce'. If the nonce is found, then <see cref="M:IResponseCookies.Delete"/> of <see cref="HttpResponse.Cookies"/> is called.</remarks>
        private string ReadNonceCookie(string nonce)
        {
            if (nonce == null)
            {
                return null;
            }

            foreach (var nonceKey in Request.Cookies.Keys)
            {
                if (nonceKey.StartsWith(Options.NonceCookie.Name))
                {
                    try
                    {
                        var nonceDecodedValue = Options.StringDataFormat.Unprotect(nonceKey.Substring(Options.NonceCookie.Name.Length, nonceKey.Length - Options.NonceCookie.Name.Length));
                        if (nonceDecodedValue == nonce)
                        {
                            var cookieOptions = Options.NonceCookie.Build(Context, Clock.UtcNow);
                            Response.Cookies.Delete(nonceKey, cookieOptions);
                            return nonce;
                        }
                    }
                    catch
                    {
                        
                    }
                }
            }

            return null;
        }

        protected override Task HandleChallengeAsync(AuthenticationProperties properties)
        {
            var publicKey = ExportPublicKey(Options.RSAKey);
            var nonce = BuildRandomString(8);

            var cookieOptions = Options.NonceCookie.Build(Context, Clock.UtcNow);

            Response.Cookies.Append(
                Options.NonceCookie.Name + Options.StringDataFormat.Protect(nonce),
                NonceProperty,
                cookieOptions);

            if (string.IsNullOrEmpty(properties.RedirectUri))
            {
                properties.RedirectUri = Request.Scheme + "://" + Request.Host +
                                         OriginalPathBase + Request.Path + Request.QueryString;
            }

            var requestParams = new Dictionary<string, string>();
            requestParams["scopes"] = "session_info";
            requestParams["client_id"] = Options.ClientId;
            requestParams["nonce"] = nonce;
            requestParams["auth_redirect"] = 
                QueryHelpers.AddQueryString(
                    Request.Scheme + "://" + Request.Host + OriginalPathBase + Options.CallbackPath,
                    "state",
                    Options.StateDataFormat.Protect(properties)
                );
            requestParams["application_name"] = Options.ApplicationName;
            requestParams["public_key"] = publicKey;

            var address = QueryHelpers.AddQueryString(new Uri(Options.DiscourseUri, "user-api-key/new").ToString(),
                requestParams);

            Response.Redirect(address);

            return Task.CompletedTask;
        }

        private static string BuildRandomString(int bytes)
        {
            var byteArray = new byte[bytes];

            using (var rng = RandomNumberGenerator.Create())
            {
                rng.GetBytes(byteArray);
            }

            return string.Concat(byteArray.Select(a => $"{a:X2}"));
        }

        // yeah, .NET still doesn't support PEM by default...

        /// <summary>
        /// Export public key from MS RSACryptoServiceProvider into OpenSSH PEM string
        /// slightly modified from https://stackoverflow.com/a/28407693
        /// </summary>
        /// <param name="csp"></param>
        /// <returns></returns>
        private static string ExportPublicKey(RSAParameters parameters)
        {
            StringWriter outputStream = new StringWriter();
            using (var stream = new MemoryStream())
            {
                var writer = new BinaryWriter(stream);
                writer.Write((byte)0x30); // SEQUENCE
                using (var innerStream = new MemoryStream())
                {
                    var innerWriter = new BinaryWriter(innerStream);
                    innerWriter.Write((byte)0x30); // SEQUENCE
                    EncodeLength(innerWriter, 13);
                    innerWriter.Write((byte)0x06); // OBJECT IDENTIFIER
                    var rsaEncryptionOid = new byte[] { 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01 };
                    EncodeLength(innerWriter, rsaEncryptionOid.Length);
                    innerWriter.Write(rsaEncryptionOid);
                    innerWriter.Write((byte)0x05); // NULL
                    EncodeLength(innerWriter, 0);
                    innerWriter.Write((byte)0x03); // BIT STRING
                    using (var bitStringStream = new MemoryStream())
                    {
                        var bitStringWriter = new BinaryWriter(bitStringStream);
                        bitStringWriter.Write((byte)0x00); // # of unused bits
                        bitStringWriter.Write((byte)0x30); // SEQUENCE
                        using (var paramsStream = new MemoryStream())
                        {
                            var paramsWriter = new BinaryWriter(paramsStream);
                            EncodeIntegerBigEndian(paramsWriter, parameters.Modulus); // Modulus
                            EncodeIntegerBigEndian(paramsWriter, parameters.Exponent); // Exponent
                            var paramsLength = (int)paramsStream.Length;
                            EncodeLength(bitStringWriter, paramsLength);
                            bitStringWriter.Write(paramsStream.GetBuffer(), 0, paramsLength);
                        }
                        var bitStringLength = (int)bitStringStream.Length;
                        EncodeLength(innerWriter, bitStringLength);
                        innerWriter.Write(bitStringStream.GetBuffer(), 0, bitStringLength);
                    }
                    var length = (int)innerStream.Length;
                    EncodeLength(writer, length);
                    writer.Write(innerStream.GetBuffer(), 0, length);
                }

                var base64 = Convert.ToBase64String(stream.GetBuffer(), 0, (int)stream.Length).ToCharArray();
                // WriteLine terminates with \r\n, we want only \n
                outputStream.Write("-----BEGIN PUBLIC KEY-----\n");
                for (var i = 0; i < base64.Length; i += 64)
                {
                    outputStream.Write(base64, i, Math.Min(64, base64.Length - i));
                    outputStream.Write("\n");
                }
                outputStream.Write("-----END PUBLIC KEY-----");
            }

            return outputStream.ToString();
        }

        /// <summary>
        /// https://stackoverflow.com/a/23739932/2860309
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="length"></param>
        private static void EncodeLength(BinaryWriter stream, int length)
        {
            if (length < 0) throw new ArgumentOutOfRangeException("length", "Length must be non-negative");
            if (length < 0x80)
            {
                // Short form
                stream.Write((byte)length);
            }
            else
            {
                // Long form
                var temp = length;
                var bytesRequired = 0;
                while (temp > 0)
                {
                    temp >>= 8;
                    bytesRequired++;
                }
                stream.Write((byte)(bytesRequired | 0x80));
                for (var i = bytesRequired - 1; i >= 0; i--)
                {
                    stream.Write((byte)(length >> (8 * i) & 0xff));
                }
            }
        }

        /// <summary>
        /// https://stackoverflow.com/a/23739932/2860309
        /// </summary>
        /// <param name="stream"></param>
        /// <param name="value"></param>
        /// <param name="forceUnsigned"></param>
        private static void EncodeIntegerBigEndian(BinaryWriter stream, byte[] value, bool forceUnsigned = true)
        {
            stream.Write((byte)0x02); // INTEGER
            var prefixZeros = 0;
            for (var i = 0; i < value.Length; i++)
            {
                if (value[i] != 0) break;
                prefixZeros++;
            }
            if (value.Length - prefixZeros == 0)
            {
                EncodeLength(stream, 1);
                stream.Write((byte)0);
            }
            else
            {
                if (forceUnsigned && value[prefixZeros] > 0x7f)
                {
                    // Add a prefix zero to force unsigned if the MSB is 1
                    EncodeLength(stream, value.Length - prefixZeros + 1);
                    stream.Write((byte)0);
                }
                else
                {
                    EncodeLength(stream, value.Length - prefixZeros);
                }
                for (var i = prefixZeros; i < value.Length; i++)
                {
                    stream.Write(value[i]);
                }
            }
        }
    }

    public class DiscourseAuthenticationOptions : RemoteAuthenticationOptions
    {
        private CookieBuilder _nonceCookieBuilder;

        public DiscourseAuthenticationOptions()
        {
            CallbackPath = "/auth-discourse";

            _nonceCookieBuilder = new DiscourseNonceCookieBuilder(this)
            {
                Name = ".CitizenFX.Discourse.Nonce.",
                HttpOnly = true,
                SameSite = SameSiteMode.None,
                SecurePolicy = CookieSecurePolicy.SameAsRequest,
                IsEssential = true,
            };

            DataProtectionProvider = Microsoft.AspNetCore.DataProtection.DataProtectionProvider.Create("FXServer");

            var dataProtector = DataProtectionProvider.CreateProtector(
                typeof(DiscourseAuthenticationHandler).FullName,
                typeof(string).FullName,
                "DAO",
                "v1");

            StringDataFormat = new SecureDataFormat<string>(new StringSerializer(), dataProtector);

            StateDataFormat = new PropertiesDataFormat(dataProtector);
        }

        public ISecureDataFormat<string> StringDataFormat;

        public ISecureDataFormat<AuthenticationProperties> StateDataFormat { get; set; }

        public Uri DiscourseUri { get; set; }

        public RSAParameters RSAKey { get; set; }

        public string ClientId { get; set; }

        public string ApplicationName { get; set; }

        public CookieBuilder NonceCookie
        {
            get => _nonceCookieBuilder;
            set => _nonceCookieBuilder = value ?? throw new ArgumentNullException(nameof(value));
        }

        private class DiscourseNonceCookieBuilder : RequestPathBaseCookieBuilder
        {
            private readonly DiscourseAuthenticationOptions _options;

            public DiscourseNonceCookieBuilder(DiscourseAuthenticationOptions daOptions)
            {
                _options = daOptions;
            }

            protected override string AdditionalPath => _options.CallbackPath;

            public override CookieOptions Build(HttpContext context, DateTimeOffset expiresFrom)
            {
                var cookieOptions = base.Build(context, expiresFrom);

                if (!Expiration.HasValue || !cookieOptions.Expires.HasValue)
                {
                    cookieOptions.Expires = expiresFrom.AddSeconds(3600);
                }

                return cookieOptions;
            }
        }

        private class StringSerializer : IDataSerializer<string>
        {
            public string Deserialize(byte[] data)
            {
                return Encoding.UTF8.GetString(data);
            }

            public byte[] Serialize(string model)
            {
                return Encoding.UTF8.GetBytes(model);
            }
        }
    }
}