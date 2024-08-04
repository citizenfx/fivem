using System;
using System.Collections.Generic;

namespace CitizenFX.Core
{
    public class HashedBinaryHeap
    {
        private BinaryHeap<Coroutine> heap;
        private Dictionary<Coroutine, int> indices;

        public HashedBinaryHeap(int capacity)
        {
            heap = new BinaryHeap<Coroutine>(capacity);
            indices = new Dictionary<Coroutine, int>(capacity);
        }

        public int Count => heap.Count;

        public void Enqueue(Coroutine coroutine)
        {
            heap.Enqueue(coroutine);
            indices[coroutine] = heap.Count - 1;
        }

        public Coroutine Dequeue()
        {
            Coroutine coroutine = heap.Dequeue();
            indices.Remove(coroutine);
            return coroutine;
        }

        public void Remove(Coroutine coroutine)
        {
            if (indices.TryGetValue(coroutine, out int index))
            {
                heap.Dequeue();
                indices.Remove(coroutine);
            }
        }

        public Coroutine Peek()
        {
            return heap.Peek();
        }
    }
}
