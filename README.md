# [DRAFT] Apple News

## Before you start

These instructions assume that you have a working Drupal 7 site (brand new or pre-existing) ready to work with, and that you have access to administer modules and add code to the site's code base. To install Drupal locally or on remote server, please see these guides on [Quick installation](https://www.drupal.org/documentation/install) and [Quick installation for developers](https://www.drupal.org/documentation/install/developers).

[Drush](https://github.com/drush-ops/drush) is not mandatory but will make for an easier, and faster installation.


## Installation

You can install the apple_news module using Drush or manually adding the modules and libraries to your code base.

#### Drush based installation

To properly run drush commands on your site you must shell into your Drupal sites directory, into the folder that contains the settings.php file specifically for your site. Some examples of what this would be are: 
```shell
sites/default/[location of settings.php file]
sites/mysite.local/[location of settings.php file]
sites/mysuperawesomenewssite.com/[location of settings.php file]
```

To install Apple News using drush and command line: 


1. First, download and enable the module dependencies for this module.     
    -   [Libraries](https://www.drupal.org/project/libraries)
    -   [Entity](https://www.drupal.org/project/entity)
    
    The following drush command will download and enable both modules.
    
    ```shell
    drush en libraries entity -y
    ```
2. Next, in your terminal, navigate to your `sites/all/libraries` folder and run the following curl commands to download the proper library versions. If the `libraries` folder does not exist, please create it before running the following command: 
    
    ```shell
    $ curl -L https://github.com/php-curl-class/php-curl-class/archive/3.5.5.tar.gz | tar xz
    ```
     **NOTE: The AppleNews library is currently gated in a private repo. the curl command won't work until publicly released. To get around this, if you have access to the repo, you can download from [the github page](https://github.com/chapter-three/AppleNews/tree/0.1.9)**
3. Next, still inside the libraries folder, run the following curl command:

    ```shell
    $ curl -L https://github.com/chapter-three/AppleNews/archive/0.1.9.zip | tar xz
    ```
4. After the libraries are downloaded, your should see directories matching the following setups (You may need to rename the folders you just downloaded to match):

    ```shell
    sites/all/libraries/AppleNews-0.1.9
    sites/all/libraries/php-curl-class-3.5.5
    ```
    
5. Add the apple_news module to your code base in the [usual manner](https://www.drupal.org/documentation/install/modules-themes), enable it using the drush command:
**NOTE: Before the module is added to drupal.org, the apple_news module can't be downloaded using [Drush](https://github.com/drush-ops/drush). It must be manually added to the sites/all/modules/contrib directory. If you have access to the repo, you can download the module from [the github page](https://github.com/chapter-three/apple_news)**

    ```shell
drush en apple_news -y
```

If you enable the module before downloading the required libraries, you will receive error messages telling you download them. Please try re-installing the libraries or check out the [Troubleshooting Section](#troubleshooting).

After installing all modules and libraries, check the status of the installation on your sites Status Report page (`admin/reports/status`). Look for "Apple News" and "PHP Curl Class" and make sure they are green. If green, you are good to go and jump to the [Configuration Section](#configuration). If not, please see the [Troubleshooting section](#troubleshoot).

#### Manually based installation

To manually install:

1.  Download this module and its dependencies in the [usual manner](https://www.drupal.org/documentation/install/modules-themes):
    -   [Libraries](https://www.drupal.org/project/libraries)
    -   [Entity](https://www.drupal.org/project/entity)
2.  Next, [Download and install the Apple News library](https://github.com/chapter-three/AppleNews/archive/0.1.9.zip) into your libraries folder. If you do not have a sites/all/libraries folder, please create it before downloading **Note: The AppleNews library is currently gated in a private repo. This download may not be accessible unless you have access to the private repository**
3.  [Download and install the PHP Curl Class library (version 3.5.5)](https://github.com/php-curl-class/php-curl-class/archive/3.5.5.tar.gz) into your libraries folder. **Note: Must be version 3.5.5, not higher or lower.**
4. After the libraries are downloaded, you should see directories matching the following setups (You may need to rename the folders you just downloaded to match):

    ```shell
    sites/all/libraries/AppleNews-0.1.9
    sites/all/libraries/php-curl-class-3.5.5
    ```
5.  Visit `admin/modules` and enable the Apple News module. This will enable the Libraries module and the Entities module, as well as any other additional dependencies. 

After installing all modules and libraries, check the status of the installation on your sites Status Report page (`admin/reports/status`). Look for "Apple News" and "PHP Curl Class" and make sure they are green. If green, you are good to go and jump to the [Configuration Section](#configuration). If not, please see the [Troubleshooting section](#troubleshoot)


## <a name="configuration"></a>Configuration
Congrats on installing the Apple News module. Please follow these configuration instructions to start publishing your content.

#### Initial settings configuration
1. Visit apple.com to get your credentials and create a news channel that your Drupal site will use.
2. In your Drupal site, navigate to the "Apple news credentials page" (`admin/config/content/apple-news/settings`) and add your Apple News credentials.
3. In your Drupal site, navigate to the "Apple news feeds/channels page" (`admin/config/content/apple-news/settings/channels`) and add a feed ID from your apple account. Please add one ID at a time. The channels are validated by the apple credentials you added to your Drupal site, and if valid, will fetch the channel information and add them to your sites list of channels.

#### Default Feed/Export configuration
1. In your Drupal site, navigate to the "Apple news export manager page" (`admin/config/content/apple-news`).
2. Click on the **'edit'** link of the feed you would like to connect to an apple news channel. The default export that comes with the Apple News module is "Nodes", but other exports can be created in code, using custom modules. For an example module, check out the "apple_news_article" module that comes with the Apple News module.
3. In this "Edit page", the minimum requirements to properly configure a feed to an apple news channel are:
    1. Under "Add new component", select "Body(apple_news)".
    2. Under "Feeds", select the Feed (Apple News Channel) that this export will be tied to. (in other words, this export feed will get nodes, process them, and send them to this selected channel for display in the Apple News app.)
    3. Under "Content types", select the content types that should be processed with this feed.
    4. Under "Layout", the default value is "Simple", but it is possible to create new layouts using a custom module.
    5. Click **Save Changes**
    6. After saving, you will see some options to the right of the new components we just added. These are **"edit"** and **"delete"**. Click on **"edit"**
    7. Select the source field for this component. This is where we tell apple news that this apple news component will be getting it's data from this Drupal field.
    8. Click **Save Changes**

#### Node configuration
Once a content type is enabled in an export/feed, the option to add the individual post reside in the nodes add/edit page. If a content type is not added to any feed export, these options will not be available on the node add/edit page.

1. To add a node to the feed sent to apple, In the "Apple News" tab, select one or more feeds from the available list. 
2. And for each selected feed, select an available "Section" that it belongs to. ("Sections" are created on apple.com, where you initially created the channel).
3. Once a node is initially published to an apple news channel, It will also display a general information section showing Post date, Share URL, the Section, and the Channel its published to.

#### Preview a post before publishing
If you want to preview a post before sending it to apple, You will need to first download and install the Apple "News Preview" Application (LINK TBD).

1. After saving the node, return to the nodes edit page
2. Find the "Apple News" Tab, and click the "Download" link under "Preview". This will download a folder containing the specialy formatted file needed by the News Preview App.
2. Drag the whole folder into the App icon to open, and it will display the page just as the Apple News App will be displaying it.

#### Delete a post from publishing
If you want to delete a post from a channel, but not delete the post itself, There is a **delete** link in the "Apple News" tab.


###Congrats!
You are now ready to start sharing your posts and articles with the Apple News Service and with the world. Happy Posting!


## <a name="troubleshoot"></a>Troubleshoot

If you are having trouble installing the module or it's dependencies, review the common scenarios below.

(Troubleshooting section under works. 
Creating list of common issues as we continue development).


## Run Tests

To run Drupal testing, Enable the core "Testing" module, from the Modules admin page or with command line.

To enable and run tests from the UI:

1. Navigate to the Testing admin page (`admin/config/development/testing`).
2. Select "Apple News" from the list of tests
3. Click **Run Tests**

To run test from command line, enter the following commands one at a time:

```shell
drush -y en simpletest
php scripts/run-tests.sh --verbose --color 'Apple News'
```

