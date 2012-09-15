/** @file libdictionary.c */
/* 
 * CS 241
 * The University of Illinois
 */

#define _GNU_SOURCE
#include <search.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "libdictionary.h"


/** Private. */
static int compare(const void *a, const void *b)
{
	return strcmp(((const dictionary_entry_t *)a)->key, ((const dictionary_entry_t *)b)->key); 
}

/** Private. */
static dictionary_entry_t *malloc_entry_t(const char *key, const char *value)
{
	dictionary_entry_t *entry = malloc(sizeof(dictionary_entry_t));
	entry->key = key;
	entry->value = value;

	return entry;
}

/** Private. */
static dictionary_entry_t *dictionary_tfind(dictionary_t *d, const char *key)
{
	dictionary_entry_t tentry = {key, NULL};
	void *tresult = tfind((void *)&tentry, &d->root, compare);

	if (tresult == NULL)
		return NULL;
	else
		return *((dictionary_entry_t **)tresult);
}

/** Private. */
static void dictionary_tdelete(dictionary_t *d, const char *key)
{
	dictionary_entry_t tentry = {key, NULL};
	tdelete((void *)&tentry, &d->root, compare);
}

/** Private. */
static int dictionary_remove_options(dictionary_t *d, const char *key, int free_memory)
{
	dictionary_entry_t *entry = dictionary_tfind(d, key);

	if (entry == NULL)
		return NO_KEY_EXISTS;
	else
	{
		dictionary_tdelete(d, key);

		if (free_memory)
		{
			free((void *)entry->key);
			free((void *)entry->value);
		}
		free(entry);

		return 0;
	}
}

/** Private. */
static void destroy_no_element_free(void *ptr)
{
	free(ptr);
}

/** Private. */
static void destroy_with_element_free(void *ptr)
{
	dictionary_entry_t *entry = (dictionary_entry_t *)ptr;

	free((void *)entry->key);
	free((void *)entry->value);
	free(entry);
}


/**
 * Initialize the dictionary data structure. This function must be
 * called before any other libdictionary functions.
 *
 * @param d
 *    Dictionary data structure.
 */
void dictionary_init(dictionary_t *d)
{
	d->root = NULL;
	pthread_mutex_init(&d->mutex, NULL);
}


/**
 * Adds the key-value pair (key, value) to the dictionary, if and only if
 * the dictionary does not already contain a key with the same name as key.
 * This function does NOT make a copy of the key or value.
 *
 * This function is thread-safe.
 *
 * You may assume that:
 * - The stirngs key and value will not be modified outside of the dictionary.
 * - The parameters will be valid, non-NULL pointers.
 *
 * @param d
 *    Dictionary data structure.
 * @param key
 *    The key to be added to the dictionary.
 * @param value
 *    The value to be assoicated with the key in the dictionary.
 *
 * @retval 0
 *    Success
 * @retval KEY_EXISTS
 *    The dictionary already contains they specified key.
 */
int dictionary_add(dictionary_t *d, const char *key, const char *value)
{
	pthread_mutex_lock(&d->mutex);

	if (dictionary_tfind(d, key) == NULL)
	{
		tsearch((void *)malloc_entry_t(key, value), &d->root, compare);

		pthread_mutex_unlock(&d->mutex);
		return 0;
	}
	else
	{
		pthread_mutex_unlock(&d->mutex);
		return KEY_EXISTS;
	}
}


/**
 * Returns the value of the key-value element for a specific key.
 * If the key does not exist, this function returns NULL. 
 *
 * This function is thread-safe.
 *
 * You may assume that:
 * - The parameters will be valid, non-NULL pointers.
 *
 * @param d
 *   A pointer to an initalized dictionary data structure.
 * @param key
 *   The key to lookup in the dictionary.
 *   
 * @return
 *  - the value of the key-value element, if the key exists in the dictionary
 *  - NULL, otherwise.
 */
const char *dictionary_get(dictionary_t *d, const char *key)
{
	pthread_mutex_lock(&d->mutex);
	dictionary_entry_t *entry = dictionary_tfind(d, key);

	if (entry == NULL)
	{
		pthread_mutex_unlock(&d->mutex);
		return NULL;
	}
	else
	{
		pthread_mutex_unlock(&d->mutex);
		return entry->value;
	}
}


/**
 * Removes the key-value pair for a given key from the dictionary, if it exists.
 *
 * This function will not free() the key or value.
 * @see dictionary_remove_free()
 *
 * You may assume that:
 * - The parameters will be valid, non-NULL pointers.
 *
 * @param d
 *   A pointer to an initalized dictionary data structure.
 * @param key
 *   The key to remove from the dictionary.
 *   
 * @retval 0
 *   Success.
 * @retval NO_KEY_EXISTS
 *   The dictionary did not contain key.
 */
int dictionary_remove(dictionary_t *d, const char *key)
{
	pthread_mutex_lock(&d->mutex);
	int val = dictionary_remove_options(d, key, 0);
	pthread_mutex_unlock(&d->mutex);

	return val;
}


/**
 * Removes the key-value pair for a given key from the dictionary, if it exists,
 * and free()s the key and value strings for the user.
 *
 * You may assume that:
 * - The parameters will be valid, non-NULL pointers.
 *
 * @param d
 *   A pointer to an initalized dictionary data structure.
 * @param key
 *   The key to remove from the dictionary.
 *   
 * @retval 0
 *   Success.
 * @retval NO_KEY_EXISTS
 *   The dictionary did not contain key.
 */
int dictionary_remove_free(dictionary_t *d, const char *key)
{
	pthread_mutex_lock(&d->mutex);
	int val = dictionary_remove_options(d, key, 1);
	pthread_mutex_unlock(&d->mutex);

	return val;
}


/**
 * Frees any memory associated with the dictionary.
 *
 * This function does not free() any keys or values of the elements contained in
 * the dictionary. @see dictionary_destroy_free()
 *
 * You may assume that:
 * - This function will only be called once per dicitonary_t instance.
 * - This function will be the last function called on each dictionary_t instance.
 * - The dictionary pointer will be valid, non-NULL pointer.
 *
 * @param d
 *   A pointer to an initalized dictionary data structure.
 */
void dictionary_destroy(dictionary_t *d)
{
	tdestroy(d->root, destroy_no_element_free);
	d->root = NULL;

	pthread_mutex_destroy(&d->mutex);
}


/**
 * Frees any memory associated with the dictionary.  Additionally, this function
 * will free() the key and value strings of all entries that still exist in
 * the dictionary.  To free only the internal memory to the dictionary,
 * @see dictionary_destroy().
 *
 * You may assume that:
 * - This function will only be called once per dicitonary_t instance.
 * - This function will be the last function called on each dictionary_t instance.
 * - The dictionary pointer will be valid, non-NULL pointer.
 *
 * @param d
 *   A pointer to an initalized dictionary data structure.
 */
void dictionary_destroy_free(dictionary_t *d)
{
	tdestroy(d->root, destroy_with_element_free);
	d->root = NULL;

	pthread_mutex_destroy(&d->mutex);
}

