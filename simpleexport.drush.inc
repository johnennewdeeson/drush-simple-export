<?php

/**
 * @file
 * Simple export and import for Drupal content.
 */

/**
 * Implements hook_drush_command().
 */
function simpleexport_drush_command() {
  $items = array();

  $items['simpleexport'] = array(
    'description' => dt('Export content matching a specification'),
    'aliases' => array('sexp'),
    'arguments' => array(
      'entity_type' => dt('Entity types to export'),
      'conditions' => dt('Optional property conditions to extract with e.g. "uid:1" "nid:100"'),
    ),
  );

  $items['simpleimport'] = array(
    'description' => dt('Import entities exported via simple export'),
    'aliases' => array('simp'),
    'arguments' => array(
      'override' => dt('Override properties of the entities during import.
          Syntax is "property_name:new_value", e.g. "uid:10"
          Only simple string value supported.
          Unset a property by setting the value to NULL e.g. "nid:NULL"')
    ),
  );

  return $items;
}

/**
 * Simple export entities
 *
 * @param string $entity_type
 *   The entity type to export
 *
 * Examples of use:
 *   Export all nodes owned by user 218
 *   drush @test simpleexport node "uid:218" > ../backups/test-node.ent
 *
 *   Export all files owned by user 218
 *   drush @test simpleexport file "uid:218" > ../backups/test-file.ent
 */
function drush_simpleexport($entity_type) {
  $args = func_get_args();
  array_shift($args);
  $conditions = _simpleexport_get_argument_array($args);

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', $entity_type);

  foreach ($conditions as $property => $value) {
    $query->propertyCondition($property, $value);
  }

  $results = $query->execute();

  if (isset($results[$entity_type])) {
    $entities = entity_load($entity_type, array_keys($results[$entity_type]));

    foreach ($entities as $entity) {
      $entity->entity_type = $entity_type;
      drush_print(str_replace("\n", '<[!]>', serialize($entity)));
    }
  }
}

/**
 * Simple import entities.
 *
 * examples of use:
 *   Import a bunch of exported nodes, change the owner to 38796
 *   and create new (don't update existing with same nid)
 *   drush @prod simpleimport "uid:38796" "nid:NULL" < ../backups/test-node.ent
 *
 *   Import a bunch of files which do exist on target with same fid as source
 *   but change owner to 38796
 *   drush @prod simpleimport "uid:38796" < ../backups/test-file.ent
 */
function drush_simpleexport_simpleimport() {
  $overrides = _simpleexport_get_argument_array(func_get_args());
  $errors = array();

  while (!feof(STDIN)) {
    try {
      $serialized_string = str_replace('<[!]>', "\n", fgets(STDIN));

      if (empty($serialized_string)) {
        continue;
      }

      $entity = unserialize($serialized_string);

      if (empty($entity) || !is_object($entity) || empty($entity->entity_type)) {
        continue;
      }

      $entity_type = $entity->entity_type;
      unset($entity->entity_type);

      $info = entity_get_info($entity_type);
      $id_field = $info['entity keys']['id'];
      drush_print('Going to import entity with ' . $id_field . ' ' . $entity->{$id_field});

      $entity_wrapper = entity_metadata_wrapper($entity_type, $entity);

      foreach ($overrides as $property => $value) {
        if ($value === 'NULL') {
          unset($entity->{$property});
        }
        elseif (in_array($property, $info['schema_fields_sql']['base table'])) {
          $entity->{$property} = $value;
        }
        else {
          $entity_wrapper->{$property} = $value;
        }
      }

      $entity_wrapper->save();

      drush_print( '-- new entity has ' .  $id_field . ' ' . $entity->{$id_field});
    }
    catch (EntityMetadataWrapperException $e) {
      $errors[] = $e->getMessage();
    }
  }

  foreach ($errors as $error_msg) {
    drush_set_error($error_msg);
  }
}

/**
 * Format drush arguments to array.
 *
 * @param array $args
 *   An array of function arguments which are strings with a colon in them
 *   separating key:value
 *
 * @return array
 *   Key is the keys and value, the value.
 */
function _simpleexport_get_argument_array($args) {
  $return = array();

  foreach ($args as $arg) {
    $parts = explode(':', $arg);
    $return[$parts[0]] = $parts[1];
  }

  return $return;
}
