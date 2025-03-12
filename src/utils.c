#include "../include/utils.h"

char *obs_array_to_string(obs_data_array_t *array)
{
	size_t array_size = obs_data_array_count(array);

	if (array == NULL || array_size == 0) {
		return "[]";
	}

	// Estimate maximum size for the string (2 chars per element + commas + brackets)
	size_t estimated_size = array_size * 2 + array_size - 1 + 2; // Two chars per element + commas + brackets
	char *result = (char *)malloc(estimated_size * sizeof(char));

	// Start the string with the opening bracket
	strcpy(result, "[");

	// Loop through the array and append each element
	for (size_t i = 0; i < array_size; ++i) {
		// Convert element to string (single character)
		obs_data_t *data = obs_data_array_item(array, i);
		const char *element[2] = {obs_data_get_string(data, "value"), '\0'}; // Create a single char string

		strcat(result, *element);

		// Add a comma if not the last element
		if (i < array_size - 1) {
			strcat(result, ", ");
		}
	}

	// End the string with the closing bracket
	strcat(result, "]");

	return result;
	return "[]";
}

// Function to initialize the dynamic string array
void init_string_array(StringArray *string_array, size_t initial_capacity)
{
	string_array->data = (char **)malloc(initial_capacity * sizeof(char *));
	string_array->size = 0;
	string_array->capacity = initial_capacity;
}

// Function to add a string to the array
void add_string(StringArray *string_array, const char *str)
{
	// Resize if needed
	if (string_array->size >= string_array->capacity) {
		string_array->capacity *= 2;
		string_array->data = (char **)realloc(string_array->data, string_array->capacity * sizeof(char *));
	}

	// Allocate memory for the new string and copy it
	string_array->data[string_array->size] = strdup(str); // strdup() allocates and copies the string
	string_array->size++;
}

// Function to get a string by index
const char *get_string(const StringArray *string_array, size_t index)
{
	if (index >= string_array->size)
		return NULL; // Out of bounds
	return string_array->data[index];
}

// Function to free the dynamic string array
void free_string_array(StringArray *string_array)
{
	for (size_t i = 0; i < string_array->size; i++) {
		free(string_array->data[i]); // Free each string
	}
	free(string_array->data); // Free the array of pointers
}

char *stringify_string_array(const StringArray *string_array, size_t threshold, const char *indent)
{
	if (string_array->size == 0)
		return strdup("[]"); // Return empty brackets if no elements

	// Calculate the initial length of the compact format
	size_t total_length = 3; // For "[" and "]\0"
	for (size_t i = 0; i < string_array->size; i++) {
		total_length += strlen(string_array->data[i]) + 4; // Quotes, comma, space
	}

	// Allocate memory for the final string (before prettification)
	char *result = (char *)malloc(total_length * sizeof(char));
	if (!result)
		return NULL; // Handle allocation failure

	// Apply compact format first
	strcpy(result, "[");
	for (size_t i = 0; i < string_array->size; i++) {
		strcat(result, "\"");
		strcat(result, string_array->data[i]);
		strcat(result, "\"");
		if (i < string_array->size - 1)
			strcat(result, ", ");
	}
	strcat(result, "]");

	// If the compact string exceeds the threshold, prettify the result
	if (strlen(result) > threshold) {
		size_t prettified_length = 3 + (string_array->size - 1) * strlen(indent) +
					   (string_array->size - 1); // Account for indentation and newlines

		// Reallocate for the prettified string
		char *prettified_result = (char *)malloc(prettified_length * sizeof(char));
		if (!prettified_result) {
			free(result);
			return NULL; // Handle allocation failure
		}

		// Apply prettified format
		strcpy(prettified_result, "[\n");
		for (size_t i = 0; i < string_array->size; i++) {
			strcat(prettified_result, indent); // Add indentation before each element
			strcat(prettified_result, "\"");
			strcat(prettified_result, string_array->data[i]);
			strcat(prettified_result, "\"");

			if (i < string_array->size - 1) {
				strcat(prettified_result, ",\n");
			}
		}

		strcat(prettified_result, "\n]");
		free(result); // Free the original compact string
		return prettified_result;
	}

	return result; // Return the compact string if it's within the threshold
}