using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using CitizenFX.Core;
using Microsoft.AspNetCore.Hosting.Server;
using Microsoft.AspNetCore.Http.Features;
using Microsoft.AspNetCore.Owin;
using static CitizenFX.Core.Native.API;

namespace FxWebAdmin
{
    internal class HttpServer : IServer
    {
        private class HttpServerScript : BaseScript
        {
            public ConcurrentQueue<Tuple<Action, TaskCompletionSource<int>>> TickQueue { get; } = new ConcurrentQueue<Tuple<Action, TaskCompletionSource<int>>>();

            [Tick]
            public Task OnTick()
            {
                while (TickQueue.TryDequeue(out var call))
                {
                    try
                    {
                        call.Item1();

                        call.Item2.SetResult(0);
                    }
                    catch (Exception e)
                    {
                        call.Item2.SetException(e);
                    }
                }

                return Task.CompletedTask;
            }
        }

        private static HttpServerScript m_ticker;

        public HttpServer()
        {
            m_ticker = new HttpServerScript();

            BaseScript.RegisterScript(m_ticker);
        }

        public static Task QueueTick(Action action)
        {
            var tcs = new TaskCompletionSource<int>();
            m_ticker.TickQueue.Enqueue(Tuple.Create(action, tcs));

            return tcs.Task;
        }

        public IFeatureCollection Features { get; } = new FeatureCollection();

        public void Dispose()
        {
            
        }

        public Task StartAsync<TContext>(IHttpApplication<TContext> application, CancellationToken cancellationToken)
        {
            SetHttpHandler(new Action<dynamic, dynamic>(async (req, res) =>
            {
                var resourceName = GetCurrentResourceName();

                var bodyStream = (req.method != "GET" && req.method != "HEAD")
                        ? await GetBodyStream(req)
                            : Stream.Null;

                var oldSc = SynchronizationContext.Current;
                SynchronizationContext.SetSynchronizationContext(new SynchronizationContext());

                await Task.Factory.StartNew(async () => 
                {
                    var owinEnvironment = new Dictionary<string, object>();
                    owinEnvironment["owin.RequestBody"] = bodyStream;

                    var headers = new HeaderDictionary();

                    foreach (var headerPair in req.headers)
                    {
                        headers.Add(headerPair.Key, new string[] { headerPair.Value.ToString() });
                    }

                    owinEnvironment["owin.RequestHeaders"] = headers;

                    owinEnvironment["owin.RequestMethod"] = req.method;
                    owinEnvironment["owin.RequestPath"] = req.path.Split('?')[0];
                    owinEnvironment["owin.RequestPathBase"] = "/" + resourceName;
                    owinEnvironment["owin.RequestProtocol"] = "HTTP/1.0";
                    owinEnvironment["owin.RequestQueryString"] = (req.path.Contains('?')) ? req.path.Split('?', 2)[1] : "";
                    owinEnvironment["owin.RequestScheme"] = "http";

                    var outStream = new MemoryStream();
                    owinEnvironment["owin.ResponseBody"] = outStream;

                    var outHeaders = new Dictionary<string, string[]>();
                    owinEnvironment["owin.ResponseHeaders"] = outHeaders;

                    owinEnvironment["owin.CallCancelled"] = new CancellationToken();
                    owinEnvironment["owin.Version"] = "1.0";

                    var ofc = new FxOwinFeatureCollection(owinEnvironment);
                    var context = application.CreateContext(new FeatureCollection(ofc));

                    try
                    {
                        await application.ProcessRequestAsync(context);
                        await ofc.InvokeOnStarting();
                    }
                    catch (Exception ex)
                    {
                        Debug.WriteLine($"Exception while handling request. {ex}");

                        await ofc.InvokeOnCompleted();

                        application.DisposeContext(context, ex);

                        await QueueTick(() =>
                        {
                            res.writeHead(500);
                            res.write("Error.");
                            res.send();
                        });

                        return;
                    }

                    application.DisposeContext(context, null);

                    var realOutHeaders = owinEnvironment["owin.ResponseHeaders"] as IDictionary<string, string[]>;

                    await QueueTick(() =>
                    {
                        res.writeHead(owinEnvironment.ContainsKey("owin.ResponseStatusCode") ? (int)owinEnvironment["owin.ResponseStatusCode"] : 200, 
                            realOutHeaders.ToDictionary(a => a.Key, a => a.Value));
                    
                        res.write(outStream.ToArray());

                        res.send();
                    });

                    await ofc.InvokeOnCompleted();
                }, CancellationToken.None, TaskCreationOptions.None, TaskScheduler.FromCurrentSynchronizationContext());

                SynchronizationContext.SetSynchronizationContext(oldSc);
            }));

            return Task.CompletedTask;
        }

        public Task StopAsync(CancellationToken cancellationToken)
        {
            return Task.CompletedTask;
        }

        private async Task<Stream> GetBodyStream(dynamic req)
        {
            var tcs = new TaskCompletionSource<byte[]>();

            req.setDataHandler(new Action<byte[]>(data =>
            {
                tcs.SetResult(data);
            }), "binary");

            var bytes = await tcs.Task;
            return new MemoryStream(bytes);
        }
    }

    internal class DummyAsyncResult : IAsyncResult
	{
		public object AsyncState => null;

		public System.Threading.WaitHandle AsyncWaitHandle => null;

		public bool CompletedSynchronously => false;

		public bool IsCompleted => false;
	}

    internal class BodyStream : Stream
    {
        private MemoryStream m_baseStream;
        private dynamic req;

        public BodyStream(dynamic req)
        {
            this.req = req;
        }

        public override bool CanRead => true;

        public override bool CanSeek => true;

        public override bool CanWrite => false;

        public override long Length
        {
            get
            {
                EnsureRead();

                return m_baseStream.Length;
            }
        }

        public override long Position { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }

        public override void Flush()
        {
            
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            EnsureRead();

            return m_baseStream.Read(buffer, offset, count);
        }

        public override async Task<int> ReadAsync(byte[] buffer, int offset, int count, CancellationToken cancellationToken)
        {
            await EnsureReadAsync();

            return await m_baseStream.ReadAsync(buffer, offset, count, cancellationToken);
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            EnsureRead();

            return m_baseStream.Seek(offset, origin);
        }

        public override void SetLength(long value)
        {
            
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            
        }

        private void EnsureRead()
        {
            if (m_baseStream == null)
            {
                EnsureReadAsync().Wait();
            }
        }

        private async Task EnsureReadAsync()
        {
            if (m_baseStream != null)
            {
                return;
            }

            Debug.WriteLine("EnsureReadAsync entry");

            await Task.Factory.FromAsync(BeginWait, EndWait, null);

            Debug.WriteLine("EnsureReadAsync completed!");
        }

        private IAsyncResult BeginWait(AsyncCallback callback, object state)
        {
            HttpServer.QueueTick(() =>
            {
                this.req.setDataHandler(new Action<byte[]>(data =>
                {
                    m_baseStream = new MemoryStream(data);

                    callback(new DummyAsyncResult());
                }), "binary");
            });

            return new DummyAsyncResult();
        }

        private void EndWait(IAsyncResult result)
        {

        }
    }

    internal class FxOwinFeatureCollection : OwinFeatureCollection, IHttpResponseFeature
    {
        private Func<Task> m_onStarting;
        private Func<Task> m_onCompleted;

        public FxOwinFeatureCollection(IDictionary<string, object> environment)
            : base(environment)
        {

        }

        void IHttpResponseFeature.OnStarting(Func<object, Task> callback, object state)
        {
            m_onStarting = () =>
            {
                return callback(state);
            };
        }

        void IHttpResponseFeature.OnCompleted(Func<object, Task> callback, object state)
        {
            m_onCompleted = () =>
            {
                return callback(state);
            };
        }

        public Task InvokeOnCompleted()
        {
            return m_onCompleted?.Invoke() ?? Task.CompletedTask;
        }

        public Task InvokeOnStarting()
        {
            return m_onStarting?.Invoke() ?? Task.CompletedTask;
        }
    }

    internal class HeaderDictionary : IDictionary<string, string[]>
    {
        private Dictionary<string, string[]> m_backingDict;

        public HeaderDictionary()
        {
            m_backingDict = new Dictionary<string, string[]>();
        }

        private string NormalizeKey(string key)
        {
            return key.ToLower();
        }

        public string[] this[string key]
        {
            get => m_backingDict[NormalizeKey(key)];
            set => m_backingDict[NormalizeKey(key)] = value;
        }

        public ICollection<string> Keys => m_backingDict.Keys;

        public ICollection<string[]> Values => m_backingDict.Values;

        public int Count => m_backingDict.Count;

        public bool IsReadOnly => false;

        public void Add(string key, string[] value)
        {
            m_backingDict.Add(NormalizeKey(key), value);
        }

        public void Add(KeyValuePair<string, string[]> item)
        {
            m_backingDict.Add(NormalizeKey(item.Key), item.Value);
        }

        public void Clear()
        {
            m_backingDict.Clear();
        }

        public bool Contains(KeyValuePair<string, string[]> item)
        {
            return m_backingDict.Contains(new KeyValuePair<string, string[]>(NormalizeKey(item.Key), item.Value));
        }

        public bool ContainsKey(string key)
        {
            return m_backingDict.ContainsKey(NormalizeKey(key));
        }

        public void CopyTo(KeyValuePair<string, string[]>[] array, int arrayIndex)
        {
            throw new NotImplementedException();
        }

        public IEnumerator<KeyValuePair<string, string[]>> GetEnumerator()
        {
            return m_backingDict.GetEnumerator();
        }

        public bool Remove(string key)
        {
            return m_backingDict.Remove(NormalizeKey(key));
        }

        public bool Remove(KeyValuePair<string, string[]> item)
        {
            throw new NotImplementedException();
        }

        public bool TryGetValue(string key, out string[] value)
        {
            return m_backingDict.TryGetValue(NormalizeKey(key), out value);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_backingDict.GetEnumerator();
        }
    }
}