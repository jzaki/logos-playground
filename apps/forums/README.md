# Anonymous Private Forums

## Functional requirements

Note: here "subscribed" and "topic" refer to app layer terminology.
The first version is just text and ephemeral. V2 uses storage for larger sized content (images) and backing up/restoring chats

### Forums

- A user installs the forum Basecamp app, which has an initial default forum called "Logos Forum"
- V2: They can add/create other forums by name (or alternatively clone the forum app and change the default name).

### Posts

- A user will receive posts to forums they're subscribed to ("Logos Forum" is initially subscribed to)
- A user can send a few types of public messages to the forum:

| Type      | Topic | Text | Img (v2) | reply | Author |
| --------- | ----- | ---- | -------- | ----- | ------ |
| New topic | New   | post | opt.     | NA    | sig    |
| Old topic | (id)  | post | opt.     | opt.  | sig    |

- Public posts directed to a subscribed forum are received and stored locally to render in a UI
- V2: Images
- V2: Private posts can also be sent to a forum, encrypted for a particular group

### Identity (v2?)

- A user has one (v2: or many) unique identifiers
- Posts are signed
- On installation, the app generates a new id for the user
  - A user may wish to import their own id
- It should be practically impossible for a user to create the identity of another user

### V2: Post history

- A user can browse past posts that were sent when they were not subscribed to receive them

## Arch/components

NB: Based on current state of modules depended on, will evolve with updates to functionality & documentation.

### Overview

Like logos-chat, but public with shared messaging topics to subscribe to.

### Forums (aka "cells")

Each forum should start with a name, this can be used with a domain prefix to uniquely specify messages on the network intended for a forum.
For example: hash("Forum") + "Logos Forum" can be an initial forum people join, and discover other forums.

### Posts

- created locally and shared on the forum topic
- To begin a all posts can be shared to: hash("Forum") + "Logos Forum"
  - Consider only sending new forum topics to entire forum's messaging topic
  - Then posts to existing forum topics sent to: hash("Forum") + "Logos Forum"+ "Forum topic"

### Identity

- public/private identity key pair
- Posts signed by private key so post authors can be verified

### V2: Post History

- Data blobs of past msgs per topic saved to storage
- app can participate in backup
- A form of pagination to incrementally trace back msg history
