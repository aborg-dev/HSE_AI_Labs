// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class PongGame : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath( Path.Combine( ModulePath, "../ThirdParty/" ) ); }
    }

    private string MsgPackPath
    {
        get { return Path.Combine( ThirdPartyPath, "msgpack-c", "include" ); }
    }

    private string AsioPath
    {
        get { return Path.Combine( ThirdPartyPath, "asio", "include" ); }
    }

	public PongGame(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RenderCore", "ShaderCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Turn on exceptions, as msgpack needs them.
        UEBuildConfiguration.bForceEnableExceptions = true;

        System.Console.WriteLine("Adding include path for msgpack: " + MsgPackPath);
        PublicIncludePaths.Add(MsgPackPath);

        System.Console.WriteLine("Adding include path for asio: " + AsioPath);
        PublicIncludePaths.Add(AsioPath);
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
