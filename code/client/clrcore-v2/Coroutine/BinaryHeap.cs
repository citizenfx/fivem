using System;
using System.Collections.Generic;

namespace CitizenFX.Core
{
    public class BinaryHeap<T> where T : IComparable<T>
    {
        private T[] heap;
        private int count;

        public BinaryHeap(int capacity = 16)
        {
            heap = new T[capacity];
        }

        public int Count => count;

        public void Enqueue(T item)
        {
            if (count == heap.Length)
            {
                Resize(2 * heap.Length);
            }

            heap[count] = item;
            HeapifyUp(count);
            count++;
        }

        public T Dequeue()
        {
            if (count == 0)
            {
                throw new InvalidOperationException("Heap is empty");
            }

            T item = heap[0];
            heap[0] = heap[count - 1];
            count--;
            HeapifyDown(0);

            if (count < heap.Length / 4)
            {
                Resize(heap.Length / 2);
            }

            return item;
        }

        public T Peek()
        {
            if (count == 0)
            {
                throw new InvalidOperationException("Heap is empty");
            }

            return heap[0];
        }

        private void HeapifyUp(int index)
        {
            while (index > 0)
            {
                int parentIndex = (index - 1) / 2;
                if (heap[parentIndex].CompareTo(heap[index]) <= 0)
                {
                    break;
                }

                Swap(parentIndex, index);
                index = parentIndex;
            }
        }

        private void HeapifyDown(int index)
        {
            while (true)
            {
                int leftChildIndex = 2 * index + 1;
                int rightChildIndex = 2 * index + 2;
                int smallest = index;

                if (leftChildIndex < count && heap[leftChildIndex].CompareTo(heap[smallest]) < 0)
                {
                    smallest = leftChildIndex;
                }

                if (rightChildIndex < count && heap[rightChildIndex].CompareTo(heap[smallest]) < 0)
                {
                    smallest = rightChildIndex;
                }

                if (smallest == index)
                {
                    break;
                }

                Swap(smallest, index);
                index = smallest;
            }
        }

        private void Swap(int i, int j)
        {
            T temp = heap[i];
            heap[i] = heap[j];
            heap[j] = temp;
        }

        private void Resize(int capacity)
        {
            T[] newHeap = new T[capacity];
            Array.Copy(heap, newHeap, count);
            heap = newHeap;
        }
    }
}
