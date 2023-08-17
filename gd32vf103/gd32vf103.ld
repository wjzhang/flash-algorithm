ENTRY(_start)

SECTIONS
{
    .text : {
        *(.text)
        *(.text*)
        __etext = .;
    }
}