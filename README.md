drush-simple-export
===================

h3. Introduction

Some simple drush commands to help you export content from a site and import it into another.
The two sites should be functionaly identical, better tools exist for migrating content between different sites.

The idea of this tool is to work like mysqldump but for Drupal content. We want to dump an amount of site content
out into a file with an export command. Then import that file onto another site by piping it into an import command.

h3. Use case

The main use case if for recovery of small amounts of content from a backup for reintroduce back into the main site.

For example:

1. On Tuesday content is accidentally deleted from the site.
2. On Thursday the site owner notices their mistake.
3. There are nightly db backups but none can be reapplied without losing all content created between Tuesday and Thursday.
4. There is a staging site where a db backup with the missing data can be restored.
5. Use this tool to export the missing content from the stage site with the backup and import into the live site.

h3. Warning

This is a poweruser tool. A knowledge of drush and drush aliases is assumed. With great power comes great responsibility.
Do not use this tool lightly.  Test it to understand what it does. Take backups before running. Use at a time of light site usge.

What this tool does not do:

* Export and import files (e.g. example.pdf) but it will transfer the file entity drupal data
* Restore references, except in the case of the referencer being a deleted entity you are restoring and the referencee is an existing entity which was not deleted.
* Restore menu links. Menus need to be manually recreated.

h3. Installation

Copy the simpleexport.drush.inc file somewhere drush can find it. For example, in your sites/all/drush directory. It needs to be there
on all your sites you are going to run this command on.

h3. Exporting

Here are some examples of exporting content from the site with drush alias @test.

Export all nodes owned by user 100 to a file in the backups directory.
drush @test simpleexport node "uid:100" > ../backups/test-node.ent

Export all files owned by user 100
drush @test simpleexport file "uid:100" > ../backups/test-file.ent
(Note that the files themselves are not exported, just their entity data - you'll need to manually put the files in the right place if they were deleted.)

h3. Importing

Here are some examples of importing data from an export to a site with drush alias @prod.

Import nodes, change the owner to 200
and create new (don't update existing with same nid)
drush @prod simpleimport "uid:200" "nid:NULL" < ../backups/test-node.ent

Import a bunch of files which do exist on target with same fid as source
but change owner to 200
drush @prod simpleimport "uid:200" < ../backups/test-file.ent
