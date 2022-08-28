// https://github.com/arlenebatada/Min-Heap-header-file-in-Cpp-/blob/master/sample%20program.cpp

#ifndef MinHeap_H
#define MinHeap_H

#include <limits.h>
#include <assert.h>
#include <exception>
#include <iomanip> // setprecision

#include <config.hpp>

#define ERROR_SIZE_LESS_THAN_1 INT_MIN
#define ERROR_SIZE_INCREASED INT_MAX
#define ERROR_POSITION_GREATER_THAN_HEAP_SIZE size
#define ERROR_SIZE_DECREASED INT_MIN
#define ERROR_POSITION_LESS_THAN_0 -1
#define ERROR_HEAP_FULL capacity

namespace RTSim {
	using namespace std;

	struct MinHeapNoMinException : public exception {
		const char * what () const throw () {
			return "MinHeap: no minimum found";
		}
	};

	template<class T>
	class MinHeap //the main min heap class
	{
		private:
		/// actual dimention of heap
		int size; 

		/// max dimension of heap
		int capacity;

		/// actual elements of the heap 
		T* array;
		
		int heapify(int element_position); //heapify function is private b'coz it'll be used directly only by other member functions 
		void swap(T&, T&); //swaps two nodes of the heap
		public:

		MinHeap() {}
		
		MinHeap(T* array, int size, int capacity) //constructor which takes an array of the elements, size and capacity as parameters. 
		{
			this->array= array;
			this->size=size;
			this->capacity=capacity;
			
			if(size>=capacity) {
				return;
			}
		
			for(int i=(size-1)>>1; i>=0; i--) { //The input array is heapified. 
				heapify(i);						//Heap creation takes O(n) Time Complexity (TC) [using aggregate analysis].
			}
			
		}
		
		bool checkHeap(int from = 0) const; // ensures that the invariant of the minheap still valid
		int insert(T element);//to insert an element in the array of this heap. It takes O(log n) TC.
		int get_capacity() const; //returns the total number of elements which can be accomodated
		int getSize() const; //returns the total number of elements actually accomodated
		T   get_max() const;
		T   get_min() const;  // returns the root in O(1)
		int extract_min(); //takes out and returns the root element of the heap. Root is the minimum in a min heap. It takes O(log n) TC.
		T*  find(double key) const; // if elem is found (give only the key), it returns the whole T
		int heap_decrease_key(int element_position, int new_value); //It decreases the value of an element at a given position. It takes O(log n) TC.
		int heap_increase_key(int element_position, int new_value);//It increases the value of an element at a given position. It takes O(log n) TC.
		int heap_sort(T *output_array);//It extracts min in each iteration and copies it into the output array. This way output array is sorted with all the heap elements. It takes O(n log n) TC.
		T   show_element(int element_position) const;//returns the element present at the given position
		bool remove(T elem);
		string toString() const;
		bool _isInRange(double eval, double expected) const;

		vector<T> asVector() const {
			vector<T> res = {};
			for (int i = 0; i < getSize(); i++)
				res.push_back(show_element(i));
			return res;
		}
	};

	template<class T>
	bool MinHeap<T>::checkHeap(int i) const {
		// If a leaf node 
		if (i <= (getSize() - 2)/2) 
		    return true;
		
		// If an internal node and is greater than its children, and 
		// same is recursively true for the children 
		if (show_element(i).util <= show_element(2*i + 1).util && show_element(i).util <= show_element(2*i + 2).util && 
		    checkHeap(2*i + 1) && checkHeap(2*i + 2)) 
		    return true; 
		
		// I decided to not even return false, just abort the program  
		assert(false);
		
		return false;
	}

	template<class T>
	int MinHeap<T>::insert(T element)
	{
		if(size==capacity) //if heap is full
		{
			throw runtime_error(__func__ + string("(). MinHeap has already reached its max capacity"));
			return ERROR_HEAP_FULL;
		}
			
		array[size++] = element; //initially size=0; It increases after insertion of every element
		
		for(int i=(size-1)>>1; i>=0; i--) // heapify starts from the least non=leaf node. This is b'coz leaves are always heapified.
		{
			heapify(i);
		}

		#ifdef DEBUG
		bool found = false;
		for (int i = 0; i < getSize(); i++)
			if (show_element(i) == element) {
				found = true;
				break;
			}
		if (!found)
			throw runtime_error(__func__ + string("(). The element you wanted to inserted has not actually been inserted"));

		checkHeap();
		#endif
		
		return 0;
	}

	template<class T>
	int MinHeap<T>::get_capacity() const
	{
		return capacity;
	}

	template<class T>
	int MinHeap<T>::getSize() const
	{
		return size;
	}

	template<class T>
	T* MinHeap<T>::find(double key) const {
		T* elem = NULL;
		
		for (int i = 0; i < getSize(); i++) {
			if (key == array[i].util) {
				elem = &array[i];
				break;
			}
		}

		return elem;
	}

	template<class T>
	void MinHeap<T>::swap(T& t1, T& t2) //to swap any two data objects
	{
		T temp=t1;
		t1=t2;
		t2=temp;
	}

	template<class T>
	int MinHeap<T>::heapify(int element_position)
	{
		int right_child_position=(element_position+1)<<1; 
		int left_child_position=right_child_position-1;
		/*
			Ideally it should be element_position*2 for left child and element_position*2+1 for right child. 
			But in that implementation instead of array[0] being the root, array[1] will have to be made the root.
			So here we have (element_position+1)*2 for left child and (element_position+1)*2-1 for right child
		*/
		int smallest_element_position=element_position;
		
		if (array[left_child_position] < array[smallest_element_position] && left_child_position<size)
		{
			smallest_element_position = left_child_position;
		}
		
		if (array[right_child_position] < array[smallest_element_position] && right_child_position<size)
		{
			smallest_element_position = right_child_position;
		}
		
		if(smallest_element_position != element_position)
		{
			swap(array[smallest_element_position], array[element_position]);
			heapify(smallest_element_position);
		}
		return 0;
	}


	template<class T>
	int MinHeap<T>::extract_min()//root is the min. It is returned back
	{
		if(size < 1)
		{
			throw runtime_error(__func__ + string("(). Heap size less than 1"));
			return ERROR_SIZE_LESS_THAN_1;
		}
		
		int min= array[0];//min=root
		swap(array[0], array[size-1]);//last element is swapped with the root
		size--;//heap size is decreased coz root no longer belongs to the heap
		heapify(0);//the new root is heapified. This way there will be a new min root.

		checkHeap();

		return min;
	}

	template<class T>
	T MinHeap<T>::get_max() const {
		T max = array[0];
		for (int i = 1; i < size; i++)
			if (array[i] > max)
				max = array[i];
		return max;
	}

	template<class T>
	T MinHeap<T>::get_min() const {
		return show_element(0);
	}

	template<class T>
	int MinHeap<T>::heap_decrease_key(int element_position, int new_value)
	{
		if(size < 1)
		{
			throw runtime_error(__func__ + string("(). ERROR_POSITION_GREATER_THAN_HEAP_SIZE"));
			return ERROR_SIZE_LESS_THAN_1;
		}
		
		if(element_position > size-1)
		{
			throw runtime_error(__func__ + string("(). ERROR_POSITION_GREATER_THAN_HEAP_SIZE"));
			return ERROR_POSITION_GREATER_THAN_HEAP_SIZE;
		}
		
		if(element_position < 0)
		{
			throw runtime_error(__func__ + string("(). ERROR_POSITION_LESS_THAN_0"));
			return ERROR_POSITION_LESS_THAN_0;		
		}
		
		if(new_value > array[element_position])//if an attempt to increase the value of the element
		{
			throw runtime_error(__func__ + string("(). ERROR_SIZE_INCREASED"));
			return ERROR_SIZE_INCREASED;
		}
		
		array[element_position]=new_value;
		
		int parent_position=(element_position+1>>1)-1;
		/*
			Ideally it should be parent_position=element_position/2. 
			But in that implementation instead of array[0] being the root, array[1] will have to be made the root.
			So here we have ((element_position+1)/2) -1 for parent.
		*/
		while((array[parent_position] > array[element_position]) && (parent_position>=0))
		{
			swap(array[parent_position], array[element_position]);
			element_position = parent_position;
			parent_position=(parent_position-1)>>1;
		} 
		return 0;
	}

	template<class T>
	int MinHeap<T> :: heap_increase_key(int element_position, int new_value)
	{
		if(size < 1)
		{
			throw runtime_error(__func__ + string("(). Heap size less than 1"));
			return ERROR_SIZE_LESS_THAN_1;
		}
		
		if(element_position > size-1)
		{
			throw runtime_error(__func__ + string("(). ERROR_POSITION_GREATER_THAN_HEAP_SIZE"));
			return ERROR_POSITION_GREATER_THAN_HEAP_SIZE;
		}
		
		if(element_position < 0)
		{
			throw runtime_error(__func__ + string("(). ERROR_POSITION_LESS_THAN_0"));
			return ERROR_POSITION_LESS_THAN_0;		
		}
		
		if(new_value < array[element_position]) //if an attempt to decrease the value of the element
		{
			throw runtime_error(__func__ + string("(). ERROR_SIZE_DECREASED"));
			return ERROR_SIZE_DECREASED;
		}
		
		array[element_position]=new_value;
		
		heapify(element_position);
	}

	template<class T>
	int MinHeap<T> :: heap_sort(T *output_array)
	{
		if(size < 1)
		{
			throw runtime_error(__func__ + string("(). Heap size less than 1"));
			return ERROR_SIZE_LESS_THAN_1;
		}
		
		
		int max_loop_count=size; //extract_min will decrease 'size' each time
		
		for (int i=0; i<max_loop_count; i++)
		{
			output_array[i]=extract_min();
		}
		
	}

	template<class T>
	bool MinHeap<T>::remove(T elem)
	{
		assert(size >= 1);
		
		// safe version: remove by CPU
		bool removed = false;
		for (int i = 0; i < getSize(); i++) {
			if (array[i].cpu == elem.cpu) {
				// swap(array[i], array[size-1]);//last element is swapped with the root
				array[i] = array[size-1];
				size--;//heap size is decreased coz root no longer belongs to the heap
				heapify(i); // heapify from i down the tree
				removed = true;
				break;
			}	
		}

		#ifdef DEBUG
		bool found = false;
		for (int i = 0; i < getSize(); i++)
			if (show_element(i).cpu == elem.cpu) {
				found = true;
				break;
			}
		if (found)
			throw runtime_error(__func__ + string("(). The element you wanted to remove has not actually been removed"));

		checkHeap();
		#endif
		return removed;


		// efficient and general version:
		// bool removed = false;
		// for (int i = 0; i < getSize(); i++) {
		// 	if (array[i] == elem) {
		// 		// swap(array[i], array[size-1]);//last element is swapped with the root
		// 		array[i] = array[size-1];
		// 		size--;//heap size is decreased coz root no longer belongs to the heap
		// 		heapify(0);//the new root is heapified. This way there will be a new min root.
		// 		removed = true;
		// 		break;
		// 	}	
		// }

		// #ifdef DEBUG
		// bool found = false;
		// for (int i = 0; i < getSize(); i++)
		// 	if (show_element(i) == elem) {
		// 		found = true;
		// 		break;
		// 	}
		// if (found)
		// 	throw runtime_error(__func__ + string("(). The element you wanted to inserted has not actually been inserted"));
		// #endif
		// return removed;
	}

	template<class T>
	T MinHeap<T>::show_element(int element_position) const
	{
		if(element_position>size-1)
		{
			cerr << __func__ << "() error" << endl;
			throw runtime_error(__func__ + string("(). you gave a position behind the heap size"));
			return T();
		}
		
		#ifdef DEBUG
		checkHeap();
		#endif

		return array[element_position];
	}

	template<class T>
	string MinHeap<T>::toString() const {
		stringstream ss;
		for (int i = 0; i < getSize(); i++)
			ss << std::setprecision(10) << array[i].toString() << endl;
		return ss.str();
	}

	template<class T>
	bool MinHeap<T>::_isInRange(double eval, double expected) const {
	    const double error = 5.0;

	    double min = double(eval - eval * error/100.0);
	    double max = double(eval + eval * error/100.0);

	    return expected >= min && expected <= max; 
	}

}
#endif
