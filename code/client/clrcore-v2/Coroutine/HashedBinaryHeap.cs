using System;
using System.Collections.Generic;

namespace CitizenFX.Core
{
    public class HashedBinaryHeap
    {
        private BinaryHeap<Coroutine> heap;
        private Dictionary<Coroutine, int> indices;
        private int initialCapacity;
        private int maxCapacity;
        private int minCapacity;

        public HashedBinaryHeap(int capacity)
        {
            initialCapacity = capacity;
            maxCapacity = 1024;
            minCapacity = 16;
            heap = new BinaryHeap<Coroutine>(initialCapacity);
            indices = new Dictionary<Coroutine, int>(initialCapacity);
        }

        public int Count => heap.Count;

        public void Enqueue(Coroutine coroutine)
        {
            heap.Enqueue(coroutine);
            indices[coroutine] = heap.Count - 1;
            ResizeHeap();
        }

        public Coroutine Dequeue()
        {
            Coroutine coroutine = heap.Dequeue();
            indices.Remove(coroutine);
            ResizeHeap();
            return coroutine;
        }

        public void Remove(Coroutine coroutine)
        {
            if (indices.TryGetValue(coroutine, out int index))
            {
                heap.Dequeue();
                indices.Remove(coroutine);
                ResizeHeap();
            }
        }

        public Coroutine Peek()
        {
            return heap.Peek();
        }

        private void ResizeHeap()
        {
            int currentCount = heap.Count;
            int capacity = heap.Capacity;

            if (currentCount > capacity * 0.75)
            {
                int newCapacity = Math.Min(maxCapacity, capacity * 2);
                BinaryHeap<Coroutine> newHeap = new BinaryHeap<Coroutine>(newCapacity);
                foreach (Coroutine coroutine in heap)
                {
                    newHeap.Enqueue(coroutine);
                }
                heap = newHeap;
            }
            else if (currentCount < capacity * 0.25)
            {
                int newCapacity = Math.Max(minCapacity, capacity / 2);
                BinaryHeap<Coroutine> newHeap = new BinaryHeap<Coroutine>(newCapacity);
                foreach (Coroutine coroutine in heap)
                {
                    newHeap.Enqueue(coroutine);
                }
                heap = newHeap;
            }
        }
    }
}
