using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
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
            ScheduleResourceTick("webadmin");

            return tcs.Task;
        }

        private class RefHolder<T>
        {
            public T Value { get; set; }
        }

        public static async Task<T> QueueTick<T>(Func<T> action)
        {
            var refHolder = new RefHolder<T>();

            await QueueTick(() => 
            {
                refHolder.Value = action();
            });

            return refHolder.Value;
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

                var cts = new CancellationTokenSource();
                req.setCancelHandler(new Action(() =>
                {
                    cts.Cancel();
                }));

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

                    var outStream = new HttpOutStream(owinEnvironment, res);
                    owinEnvironment["owin.ResponseBody"] = outStream;

                    var outHeaders = new Dictionary<string, string[]>();
                    owinEnvironment["owin.ResponseHeaders"] = outHeaders;

                    owinEnvironment["owin.CallCancelled"] = cts.Token;
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

                        var errorText = Encoding.UTF8.GetBytes("Error.");

                        owinEnvironment["owin.ResponseStatusCode"] = 500;
                        await outStream.WriteAsync(errorText, 0, errorText.Length);
                        await outStream.EndStream();

                        return;
                    }

                    application.DisposeContext(context, null);

                    await outStream.EndStream();

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

    internal class HttpOutStream : Stream
    {
        private Dictionary<string, object> owinEnvironment;
        private dynamic res;
        private bool headersSent = false;

        public HttpOutStream(Dictionary<string, object> owinEnvironment, dynamic res)
        {
            this.owinEnvironment = owinEnvironment;
            this.res = res;
        }

        public override bool CanRead => false;

        public override bool CanSeek => false;

        public override bool CanWrite => true;

        public override long Length => throw new NotImplementedException();

        public override long Position { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }

        public override void Flush()
        {
            FlushAsync().Wait();
        }

        public override Task FlushAsync(CancellationToken cancellationToken)
        {
            if (!headersSent)
            {
                return HttpServer.QueueTick(() =>
                {
                    EnsureHeadersSent();
                });
            }

            return Task.CompletedTask;
        }

        private void EnsureHeadersSent()
        {
            if (!headersSent)
            {
                headersSent = true;
                
                var realOutHeaders = owinEnvironment["owin.ResponseHeaders"] as IDictionary<string, string[]>;

                res.writeHead(owinEnvironment.ContainsKey("owin.ResponseStatusCode") ? (int)owinEnvironment["owin.ResponseStatusCode"] : 200, 
                realOutHeaders.ToDictionary(a => a.Key, a => a.Value));
            }
        }

        public Task EndStream()
        {
            return HttpServer.QueueTick(() =>
            {
                EnsureHeadersSent();

                res.send();
            });
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            throw new NotImplementedException();
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            throw new NotImplementedException();
        }

        public override void SetLength(long value)
        {
            throw new NotImplementedException();
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            this.WriteAsync(buffer, offset, count).Wait();
        }

        public override Task WriteAsync(byte[] buffer, int offset, int count, CancellationToken cancellationToken)
        {
            void SendBuffer()
            {
                var outBytes = new byte[count];
                Buffer.BlockCopy(buffer, offset, outBytes, 0, count);

                res.write(outBytes);
            }

            return HttpServer.QueueTick(() =>
            {
                EnsureHeadersSent();

                SendBuffer();
            });
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

    internal class FxOwinFeatureCollection : OwinFeatureCollection, IHttpResponseFeature, IHttpRequestLifetimeFeature
    {
        private List<Func<Task>> m_onStarting = new List<Func<Task>>();
        private List<Func<Task>> m_onCompleted = new List<Func<Task>>();

        public FxOwinFeatureCollection(IDictionary<string, object> environment)
            : base(environment)
        {

        }

        void IHttpResponseFeature.OnStarting(Func<object, Task> callback, object state)
        {
            m_onStarting.Add(() =>
            {
                return callback(state);
            });
        }

        void IHttpResponseFeature.OnCompleted(Func<object, Task> callback, object state)
        {
            m_onCompleted.Add(() =>
            {
                return callback(state);
            });
        }

        void IHttpRequestLifetimeFeature.Abort()
        {
            
        }

        public async Task InvokeOnCompleted()
        {
            foreach (var action in m_onCompleted)
            {
                await action();
            }
        }

        public async Task InvokeOnStarting()
        {
            foreach (var action in m_onStarting)
            {
                await action();
            }
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