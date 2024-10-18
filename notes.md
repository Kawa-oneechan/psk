# Notes

## JSON data formats

**TODO**: write schemas for all of them, maybe have everything checked against them on load (debug build only).

For now though...
1. Dates are written as two-number arrays where the *first* item is the day and the *second* item is the month. *This should be consistent!* Perhaps a special date reader could be added that can take `"06-26"` (which is indeed the other way around) and `"Jun 26"` as well as `[ 26, 6 ]`?
2. I forgot what I was gonna say here when I finished the dates thing.


## Sampler setups

### Villager body

| Source  | Sampler | Use                                          | Layers |
| ------- | ------- | -------------------------------------------- | ------ |
| 0 - 2   | 0 - 2   | Body (albedo, mix, normal)                   |        |
| 6 - 8   | 4 - 6   | Eyes                                         | 16     |
| 9 - 11  | 8 - 10  | Mouth                                        | 9¹     |
| 12 - 15 | Not yet | Accessories 1 (albedo, mix, normal, opacity) |        |
| 16 - 19 | Not yet | Accessories 2                                |        |

¹: if the model has a muzzle/beak, there are no mouth textures.

### Player body

| Source  | Sampler | Use                        | Options | Layers |
| ------- | ------- | -------------------------- | ------- | ------ |
| 0 - 2   | 0 - 2   | Body (albedo, mix, normal) |         |        |
| 3 - 5   | 4 - 6   | Nose                       | 3¹      |        |
| 6 - 8   | 8 - 10  | Cheeks                     |         | 3²     |
| 9 - 11  | 12 - 14 | Eyes                       | 26      | 16     |
| 12 - 14 | 12 - 14 | Eyes (stung)               | 26      | 16     |
| 15 - 17 | 16 - 18 | Mouth                      | 4       | 9      |
| 18 - 20 | 0 - 3³  | Hair                       | A bunch | 0      |

**TODO**: Socks, which are rendered as part of the player.

¹: not a matter of textures, the noses are different meshes.
²: player's choice.
³: hair is a different model.

### Clothing

| Source  | Sampler | Use                                               |
| ------- | ------- | ------------------------------------------------- |
| 0 - 3   | 0 - 3   | Tops *or* onepiece (albedo, mix, normal, opacity) |
| 4 - 7   | 0 - 3   | Bottoms                                           |
| 8 - 11  | 0 - 3   | Shoes                                             |
| 12 - 15 | 0 - 3   | Hat                                               |
| 16 - 19 | 0 - 3   | Glasses                                           |
| 20 - 23 | 0 - 3   | Glasses alpha?                                    |
| 24 - 27 | 0 - 3   | Accessories                                       |
| 28 - 31 | 0 - 3   | Bag                                               |

**TODO**: Look into the "glasses alpha" part. Is that actually needed?

## Coordinate system

The coordinate system is Y+ up, X+ right. For rotation, in a `glm::vec3`, X is roll, Y is pitch, and Z is yaw, or rather that's how it works for the camera. For models, Y is yaw? So consider this whole part a **WIP**.

## MSBT

Why is it still called *Message Studio Binary Text*? It's not done in Message Studio, and it's not binary. This thing needs a rename!

But whatever.

Commands are inserted like fucked-up HTML tags, like `<color:3>` or `<str:player>`.

### Pre-processed

Preprocessed commands purely replace themselves with other text and/or commands, such as `<str:...>`, which substitutes in a particular string like the player's name.

### Live

Live commands affect the text as it gets displayed, or something out of the dialogue box like the speaker's animations, such as `<delay:...>`, `<break>`, `<color:...>`, or `<emote:...>`.

### Custom

By adding entries to `msbt/content.json`, you can hook up your own preprocessed commands.

If the entry is a bare string, the command is replaced by that string. So `"foo": "bar"` means `<foo>` is replaced with `bar`.

If the entry is a string that *specifies a `.lua` file though*, that file's content will be used. The command and all its arguments (as done by splitting on `:`) will be available as the `msbt` variable. The script is expected to return a string. `msbt[1]` will be the command's name, while `msbt[0]` is nothing because Lua is dumb.

## Assorted other crap

House stuff, exterior:
* Flag
* Level
* Status
* Walls
* Roof
* Door
* Owner
* Player?
